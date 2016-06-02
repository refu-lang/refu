#include "llvm_ast.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Linker.h>

#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/string/common.h>
#include <rfbase/string/core.h>
#include <rfbase/string/conversion.h>
#include <rfbase/utils/sanity.h>
#include <rfbase/utils/bits.h>

#include <compiler_args.h>

#include <module.h>
#include <lexer/tokens.h>
#include <analyzer/analyzer.h>
#include <ast/ast_utils.h>
#include <ast/type.h>
#include <ast/block.h>
#include <ast/returnstmt.h>
#include <ast/constants.h>
#include <ast/operators.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>
#include <ast/ifexpr.h>

#include <ir/rir.h>
#include <ir/rir_expression.h>

#include <types/type_function.h>
#include <types/type_elementary.h>
#include <types/type_utils.h>
#include <types/type_comparisons.h>
#include <types/type_operators.h>
#include <types/type.h>

#include <utils/common_strings.h>

#include <backend/llvm.h>
#include "llvm_utils.h"
#include "llvm_globals.h"
#include "llvm_arrays.h"
#include "llvm_operators.h"
#include "llvm_functions.h"
#include "llvm_types.h"
#include "llvm_values.h"
#include "llvm_conversion.h"

LLVMTypeRef bllvm_type_from_type(const struct type *type,
                                 struct llvm_traversal_ctx *ctx)
{
    char *name;
    LLVMTypeRef ret = NULL;
    if (type->category == TYPE_CATEGORY_ELEMENTARY) {
        ret = bllvm_elementary_to_type(type_elementary(type), ctx);
    } else if (type->category == TYPE_CATEGORY_DEFINED) {
        RFS_PUSH();
        name = rf_string_cstr_from_buff_or_die(type_defined_get_name(type));
        ret = LLVMGetTypeByName(ctx->llvm_mod, name);
        RFS_POP();
    } else if (type_is_sumtype(type)) {
        RFS_PUSH();
        name = rf_string_cstr_from_buff_or_die(
            type_get_unique_type_str(type)
        );
        ret = LLVMGetTypeByName(ctx->llvm_mod, name);
        RFS_POP();
    } else {
        RF_CRITICAL_FAIL("Not yet implemented type");
    }

    RF_ASSERT(ret, "The above functions should never fail");
    return ret;
}

i_INLINE_INS struct LLVMOpaqueType **llvm_traversal_ctx_get_params(struct llvm_traversal_ctx *ctx);
i_INLINE_INS unsigned llvm_traversal_ctx_get_param_count(struct llvm_traversal_ctx *ctx);
i_INLINE_INS void llvm_traversal_ctx_reset_params(struct llvm_traversal_ctx *ctx);

i_INLINE_INS struct LLVMOpaqueValue **llvm_traversal_ctx_get_values(struct llvm_traversal_ctx *ctx);
i_INLINE_INS unsigned llvm_traversal_ctx_get_values_count(struct llvm_traversal_ctx *ctx);
i_INLINE_INS void llvm_traversal_ctx_reset_values(struct llvm_traversal_ctx *ctx);

struct rir *llvm_traversal_ctx_rir(struct llvm_traversal_ctx *ctx)
{
    return ctx->mod->rir;
}

static bool llvm_traversal_ctx_map_val(struct llvm_traversal_ctx *ctx,
                                       const struct rir_value *rv,
                                       void *lv)
{
    bool ret;
    RF_ASSERT(rv->category != RIR_VALUE_NIL, "Nil RIR Value should never get here");
    ret = strmap_add(&ctx->valmap, (struct RFstring*)&rv->id, lv);
    if (!ret) {
        if (errno == EEXIST) {
            RF_ERROR("Tried to add an already existing rir value string to the llvm val mapping");
        } else {
            RF_ERROR("Failed to add a rir val to llvm val mapping");
        }
    }
    return ret;
}

bool llvm_traversal_ctx_map_llvmval(struct llvm_traversal_ctx *ctx,
                                    const struct rir_value *rv,
                                    struct LLVMOpaqueValue *lv)
{
    return llvm_traversal_ctx_map_val(ctx, rv, lv);
}
bool llvm_traversal_ctx_map_llvmblock(struct llvm_traversal_ctx *ctx,
                                      const struct rir_value *rv,
                                      struct LLVMOpaqueBasicBlock *lv)
{
    return llvm_traversal_ctx_map_val(ctx, rv, lv);
}

void llvm_traversal_ctx_reset_valmap(struct llvm_traversal_ctx *ctx)
{
    strmap_clear(&ctx->valmap);
}

LLVMValueRef bllvm_cast_value_to_elementary_maybe(LLVMValueRef val,
                                                  const struct type *t,
                                                  struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_ELEMENTARY,
              "Casting only to elementary types supported for now");
    LLVMTypeRef common_type = bllvm_elementary_to_type(type_elementary(t), ctx);
    return bllvm_cast_value_to_type_maybe(val, common_type, ctx);
}

struct LLVMOpaqueValue *bllvm_compile_constant(const struct ast_constant *n,
                                               struct rir_type *type,
                                               struct llvm_traversal_ctx *ctx)
{
    int64_t int_val;
    double float_val;
    switch (ast_constant_get_type(n)) {
    case CONSTANT_NUMBER_INTEGER:
        if (!ast_constant_get_integer(n, &int_val)) {
            RF_ERROR("Failed to convert a constant num node to integer number for LLVM");
        }
        return (int_val >= 0)
            ? LLVMConstInt(bllvm_elementary_to_type(type->etype, ctx), int_val, 0)
            : LLVMConstNeg(LLVMConstInt(bllvm_elementary_to_type(type->etype, ctx), (unsigned long long) -int_val, 0));
    case CONSTANT_NUMBER_FLOAT:
        if (!ast_constant_get_float(n, &float_val)) {
            RF_ERROR("Failed to convert a constant num node to float number for LLVM");
        }
        return LLVMConstReal(LLVMDoubleType(), float_val);
    case CONSTANT_BOOLEAN:
        return LLVMConstInt(LLVMInt1Type(), ast_constant_get_bool(n), 0);
    default:
        RF_CRITICAL_FAIL("Invalid constant type");
        break;
    }
    return NULL;
}

struct LLVMOpaqueValue *bllvm_compile_literal(const struct RFstring *lit, struct llvm_traversal_ctx *ctx)
{
    return bllvm_literal_to_global_string(lit, ctx);
}

static struct LLVMOpaqueValue *bllvm_compile_objmemberat(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(
        expr->objmemberat.objmemory->type->is_pointer,
        "You can only get an index to a pointer"
    );
    LLVMValueRef llvm_mval = bllvm_value_from_rir_value_or_die(
        expr->objmemberat.objmemory,
        ctx
    );
    return bllvm_gep_to_struct(llvm_mval, expr->objmemberat.idx, ctx);
}

static struct LLVMOpaqueValue *bllvm_compile_setunionidx(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(
        expr->setunionidx.unimemory->type->is_pointer,
        "You can only set an index to a pointer"
    );
    LLVMValueRef llvm_mval = bllvm_value_from_rir_value_or_die(
        expr->setunionidx.unimemory,
        ctx
    );
    // the union index is the first member of the struct
    LLVMValueRef llvm_idx_loc = bllvm_gep_to_struct(llvm_mval, 0, ctx);
    return LLVMBuildStore(
        ctx->builder,
        bllvm_value_from_rir_value_or_die(expr->setunionidx.idx, ctx),
        llvm_idx_loc
    );
}

static struct LLVMOpaqueValue *bllvm_compile_getunionidx(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(
        expr->getunionidx.unimemory->type->is_pointer,
        "You can only get an index to a pointer"
    );
    LLVMValueRef llvm_mval = bllvm_value_from_rir_value_or_die(
        expr->getunionidx.unimemory,
        ctx
    );
    // the union index is the first member of the struct
    LLVMValueRef llvm_idx_gep = bllvm_gep_to_struct(llvm_mval, 0, ctx);
    // but since this was a gep you have to read it
    return LLVMBuildLoad(ctx->builder, llvm_idx_gep, "");
}

static struct LLVMOpaqueValue *bllvm_compile_unionmemberat(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(
        expr->unionmemberat.unimemory->type->is_pointer,
        "You can only get an index to a pointer"
    );
    LLVMValueRef llvm_mval = bllvm_value_from_rir_value_or_die(
        expr->unionmemberat.unimemory,
        ctx
    );
    // get the member location, it's index is +1, in order to skip the selector index
    LLVMValueRef llvm_member_gep = bllvm_gep_to_struct(
        llvm_mval,
        expr->unionmemberat.idx + 1,
        ctx
    );
    return llvm_member_gep;
}


static struct LLVMOpaqueValue *bllvm_compile_alloca(const struct rir_expression *expr,
                                                    struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef type = bllvm_type_from_rir_type(expr->alloca.type, ctx);
    if (expr->alloca.alloc_location == RIR_ALLOC_STACK) {
        return LLVMBuildAlloca(ctx->builder, type, "");
    }
    LLVMValueRef other_fn = LLVMGetNamedFunction(ctx->llvm_mod, "exit");
    RF_ASSERT(other_fn, "OTHER FN NOT FOUND!");
    (void)other_fn;
    LLVMValueRef malloc_fn = LLVMGetNamedFunction(ctx->llvm_mod, "malloc");
    RF_ASSERT(malloc_fn, "We should get a malloc function here");
    LLVMValueRef call_args[] = { LLVMConstInt(LLVMInt64Type(), rir_type_bytesize(expr->alloca.type), 0) };
    LLVMValueRef retval = LLVMBuildCall(ctx->builder, malloc_fn, call_args, 1, "");
    // TODO: CHECK AND HANDLE malloc failure
    // TODO: free? (ideally follow the global ownership graph and free at the end)

    // also cast the malloc value before returning
    return LLVMBuildBitCast(ctx->builder, retval, LLVMPointerType(type, 0), "");
}

struct LLVMOpaqueValue *bllvm_compile_rirexpr(
    const struct rir_expression *expr,
    struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef llvmval;
    switch(expr->type) {
    case RIR_EXPRESSION_CONSTANT:
        RF_ASSERT(expr->val.category == RIR_VALUE_CONSTANT, "Constant expression should have a constant value");
        llvmval = bllvm_compile_constant(&expr->val.constant, expr->val.type, ctx);
        break;
    case RIR_EXPRESSION_CALL:
        llvmval = bllvm_compile_functioncall(&expr->call, ctx);
        break;
    case RIR_EXPRESSION_FIXEDARR:
        llvmval = bllvm_compile_fixedarr(expr, ctx);
        break;
    case RIR_EXPRESSION_ALLOCA:
        llvmval = bllvm_compile_alloca(expr, ctx);
        break;
    case RIR_EXPRESSION_RETURN:
        // RIR expression return code should only be generated at the end of a basic block
        // which happens in: llvm_create_blockexit()
        RF_CRITICAL_FAIL("Should never get here");
        break;
    case RIR_EXPRESSION_CONVERT:
        llvmval = bllvm_compile_conversion(expr, ctx);
        break;
    case RIR_EXPRESSION_READ:
        llvmval = LLVMBuildLoad(ctx->builder, bllvm_value_from_rir_value_or_die(expr->read.memory, ctx), "");
        break;
    case RIR_EXPRESSION_WRITE:
    {
        LLVMValueRef dstmemoryval = bllvm_value_from_rir_value_or_die(expr->write.memory, ctx);
        LLVMValueRef fromval = bllvm_value_from_rir_value_or_die(expr->write.writeval, ctx);
        // Writting to a string needs special treatment right now
        if (rir_type_is_specific_elementary(expr->write.memory->type, ELEMENTARY_TYPE_STRING)) {
            bllvm_copy_string(fromval, dstmemoryval, ctx);
            llvmval = dstmemoryval;
        } else {
            llvmval = LLVMBuildStore(ctx->builder, fromval, dstmemoryval);
        }
    }
        break;
    case RIR_EXPRESSION_ADD:
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
        llvmval = bllvm_compile_rirbop(expr, ctx);
        break;
    case RIR_EXPRESSION_CMP_EQ:
    case RIR_EXPRESSION_CMP_NE:
    case RIR_EXPRESSION_CMP_GE:
    case RIR_EXPRESSION_CMP_GT:
    case RIR_EXPRESSION_CMP_LE:
    case RIR_EXPRESSION_CMP_LT:
        llvmval = bllvm_compile_comparison(expr, ctx);
        break;
    case RIR_EXPRESSION_OBJMEMBERAT:
        llvmval = bllvm_compile_objmemberat(expr, ctx);
        break;
    case RIR_EXPRESSION_SETUNIONIDX:
        llvmval = bllvm_compile_setunionidx(expr, ctx);
        break;
    case RIR_EXPRESSION_GETUNIONIDX:
        llvmval = bllvm_compile_getunionidx(expr, ctx);
        break;
    case RIR_EXPRESSION_UNIONMEMBERAT:
        llvmval = bllvm_compile_unionmemberat(expr, ctx);
        break;
    case RIR_EXPRESSION_OBJIDX:
        llvmval = bllvm_compile_objidx(expr, ctx);
        break;
    default:
        RF_CRITICAL_FAIL("Unknown rir expression type encountered at LLVM backend generation");
        break;
    }
    // add mapping from rir value id to llvm val if value exists
    if (llvmval && expr->val.category != RIR_VALUE_NIL) {
        if (!llvm_traversal_ctx_map_llvmval(ctx, &expr->val, llvmval)) {
            return NULL;
        }
    }
    return llvmval;
}

struct LLVMOpaqueModule *blvm_create_module(struct rir *rir,
                                            struct llvm_traversal_ctx *ctx,
                                            struct LLVMOpaqueModule *link_source)
{
    // temporary. Name checking should be abstracted elsewhere
    RFS_PUSH();
    const char *mod_name = rf_string_cstr_from_buff_or_die(&rir->name);
    if (!mod_name) {
        RF_ERROR("Failure to create null terminated cstring from RFstring");
        RFS_POP();
        return NULL;
    }
    ctx->llvm_mod = LLVMModuleCreateWithName(mod_name);
    RFS_POP();
    ctx->target_data = LLVMCreateTargetData(LLVMGetDataLayout(ctx->llvm_mod));

    if (!bllvm_create_global_functions(ctx)) {
        RF_ERROR("Could not create global functions");
        goto fail;
    }

    if (rf_string_equal(&rir->name, &g_str_stdlib)) {
        // create some global definitions that the stdlib should offer
        if (!bllvm_create_globals(ctx)) {
            RF_ERROR("Failed to create general globals for LLVM");
            goto fail;
        }
    } else {
        RF_ASSERT(link_source, "If module is not stdlib, linking with something is mandatory at least for now");
        // if an error occurs LLVMLinkModules() returns true ...
        if (true == LLVMLinkModules2(ctx->llvm_mod, link_source)) {
            RF_ERROR("Could not link LLVM modules");
            goto fail;
        }
    }

    // create globals
    if (!bllvm_create_module_globals(rir, ctx)) {
        RF_ERROR("Failed to create module globals for LLVM");
        goto fail;
    }

    // create module type definitions
    if (!bllvm_create_module_types(rir, ctx)) {
        RF_ERROR("Failed to create module types for LLVM");
        goto fail;
    }

    if (!bllvm_create_module_functions(rir, ctx)) {
        RF_ERROR("Failed to create module functions for LLVM");
        goto fail;
    }

    if (compiler_args_print_backend_debug(ctx->args)) {
        bllvm_mod_debug(ctx->llvm_mod, mod_name);
    }

    return ctx->llvm_mod;

fail:
    LLVMDisposeModule(ctx->llvm_mod);
    return NULL;
}
