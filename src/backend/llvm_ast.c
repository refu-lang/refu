#include "llvm_ast.h"

#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>
#include <Utils/sanity.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <compiler_args.h>

#include <ast/ast_utils.h>
#include <ast/function.h>
#include <ast/type.h>
#include <ast/block.h>
#include <ast/returnstmt.h>
#include <ast/constant_num.h>

#include <types/type_function.h>
#include <types/type_elementary.h>

#include <ir/elements.h>

#include <backend/llvm.h>

static LLVMTypeRef backend_llvm_elementary_to_type(enum elementary_type type)
{
    switch(type) {
        // LLVM does not differentiate between signed and unsigned
    case ELEMENTARY_TYPE_INT:
    case ELEMENTARY_TYPE_UINT:
        return LLVMIntType(32);// TODO: Think of how to represent size agnostic
    case ELEMENTARY_TYPE_INT_8:
    case ELEMENTARY_TYPE_UINT_8:
        return LLVMInt8Type();
    case ELEMENTARY_TYPE_INT_16:
    case ELEMENTARY_TYPE_UINT_16:
        return LLVMInt16Type();
    case ELEMENTARY_TYPE_INT_32:
    case ELEMENTARY_TYPE_UINT_32:
        return LLVMInt32Type();
    case ELEMENTARY_TYPE_INT_64:
    case ELEMENTARY_TYPE_UINT_64:
        return LLVMInt64Type();

    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Unsupported elementary type \""RF_STR_PF_FMT"\" "
                              "during LLVM conversion",
                              RF_STR_PF_ARG(type_elementary_get_str(type)));
        break;
    }
    return NULL;
}

i_INLINE_INS struct LLVMOpaqueType **llvm_traversal_ctx_get_params(struct llvm_traversal_ctx *ctx);
i_INLINE_INS unsigned llvm_traversal_ctx_get_param_count(struct llvm_traversal_ctx *ctx);


static LLVMTypeRef backend_llvm_type(const struct rir_type *type,
                                     struct llvm_traversal_ctx *ctx)
{
    //TODO: ctx not used here for now. If not used at all remove
    (void)ctx;
    return backend_llvm_elementary_to_type(type->elementary);
}

static LLVMTypeRef *backend_llvm_types(struct rir_type *type,
                                       struct llvm_traversal_ctx *ctx)
{
    struct rir_type **subtype;
    if (darray_size(type->subtypes) == 0) {
        darray_append(ctx->params, backend_llvm_type(type, ctx));
    } else {
        darray_foreach(subtype, type->subtypes) {
            darray_append(ctx->params, backend_llvm_type(*subtype, ctx));
        }
    }
    return llvm_traversal_ctx_get_params(ctx);
}

static void backend_llvm_expression_iterate(struct ast_node *n,
                                            struct llvm_traversal_ctx *ctx)
{
    uint64_t val;
    switch(n->type) {
    case AST_RETURN_STATEMENT:
        LLVMBuildRet(ctx->builder, ctx->current_value);
        break;
    case AST_CONSTANT_NUMBER:
        if (!ast_constantnum_get_integer(n, &val)) {
            RF_ERROR("Failed to convert a constant num node to number for LLVM");
        }
        // TODO: This is not using rir_types
        ctx->current_value = LLVMConstInt(LLVMInt32Type(), val, 0);
        break;
    default:
        break;
    }
}

static void backend_llvm_expression(struct ast_node *n,
                                    struct llvm_traversal_ctx *ctx)
{
    ctx->current_value = NULL;
    backend_llvm_expression_iterate(n, ctx);
    ctx->current_value = NULL;
}

static void llvm_symbols_iterate_cb(struct symbol_table_record *rec,
                                    struct llvm_traversal_ctx *ctx)
{
    char *name;
    // for each symbol in the allocate an LLVM variable in the stack with alloca
    struct rir_type *type = symbol_table_record_rir_type(rec);
    RFS_buffer_push();
    name = rf_string_cstr_from_buff(symbol_table_record_id(rec));
    // note: this simply creates the stack space but does not allocate it
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, backend_llvm_type(type, ctx), name);
    symbol_table_record_set_backend_handle(rec, allocation);
    RFS_buffer_pop();
}

static void backend_llvm_basic_block(struct rir_basic_block *block,
                                     struct llvm_traversal_ctx *ctx)
{
    struct ast_node *expr;
    symbol_table_iterate(block->symbols, (htable_iter_cb)llvm_symbols_iterate_cb, ctx);
    rf_ilist_for_each(&block->lh, expr, ln_for_rir_blocks) {
        backend_llvm_expression(expr, ctx);
    }

}

static LLVMValueRef backend_llvm_function(struct rir_function *fn,
                                          struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef llvm_fn;
    char *fn_name;
    char *param_name;
    bool at_first;
    RFS_buffer_push();
    fn_name = rf_string_cstr_from_buff(&fn->name);
    llvm_fn = LLVMAddFunction(ctx->mod, fn_name,
                              LLVMFunctionType(backend_llvm_type(fn->ret_type, ctx),
                                               backend_llvm_types(fn->arg_type, ctx),
                                               llvm_traversal_ctx_get_param_count(ctx),
                                               0)); // never variadic for now
    RFS_buffer_pop();

    // now handle function body
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(llvm_fn, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    // place function's argument in the stack
    unsigned int i = 0;
    LLVMValueRef allocation;
    const struct RFstring *param_name_str;
    struct symbol_table_record *rec;
    for (i = 0; i <LLVMCountParams(llvm_fn); ++i) {
        // for each argument of the function allocate an LLVM variable
        // in the stack with alloca
        param_name_str = rir_type_get_nth_name(fn->arg_type, i);
        RFS_buffer_push();
        param_name = rf_string_cstr_from_buff(param_name_str);
        allocation = LLVMBuildAlloca(ctx->builder,
                                     backend_llvm_type(rir_type_get_nth_type(fn->arg_type, i), ctx),
                                     param_name);
        RFS_buffer_pop();
        // and assign to it the argument value
        LLVMBuildStore(ctx->builder, LLVMGetParam(llvm_fn, i) ,allocation);
        // also note the alloca in the symbol table
        rec = symbol_table_lookup_record(fn->symbols, param_name_str, &at_first);
        RF_ASSERT_OR_CRITICAL(rec, "Symbol table of rir_function did not contain expected parameter");
        symbol_table_record_set_backend_handle(rec, allocation);
    }

    // now handle function entry block
    backend_llvm_basic_block(fn->entry, ctx);
    return llvm_fn;
}

struct LLVMOpaqueModule *backend_llvm_create_module(struct rir_module *mod,
                                                    struct llvm_traversal_ctx *ctx)
{
    struct rir_function *fn;
    // for each function of the module create code
    rf_ilist_for_each(&mod->functions, fn, ln_for_module) {
        backend_llvm_function(fn, ctx);
    }
    return ctx->mod;
}
