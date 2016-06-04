#include "llvm_functions.h"
#include "llvm_ast.h"

#include <llvm-c/Core.h>

#include <rfbase/string/common.h>
#include <rfbase/string/conversion.h>

#include <module.h>
#include <analyzer/symbol_table.h>
#include <analyzer/analyzer.h>
#include <ast/function.h>
#include <ast/type.h>
#include <types/type.h>
#include <types/type_function.h>
#include <ir/rir_function.h>
#include <ir/rir_block.h>
#include <ir/rir.h>

#include "llvm_ast.h"
#include "llvm_utils.h"
#include "llvm_values.h"

LLVMValueRef bllvm_compile_functioncall(const struct rir_call *call,
                                        struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef ret;
    RFS_PUSH();
    LLVMValueRef llvm_fn = LLVMGetNamedFunction(
        ctx->llvm_mod,
        rf_string_cstr_from_buff_or_die(&call->name)
    );
    RFS_POP();
    if (!llvm_fn) {
        RF_ERROR("Could not find an llvm function by name");
        return NULL;
    }
    bllvm_value_arr_to_values(&call->args, ctx); // can be NULL if there are no args
    ret = LLVMBuildCall(
        ctx->builder,
        llvm_fn,
        llvm_traversal_ctx_get_values(ctx),
        llvm_traversal_ctx_get_values_count(ctx),
        ""
    );
    return ret;
}

LLVMTypeRef bllvm_function_type(LLVMValueRef fn)
{
    return LLVMGetElementType(LLVMTypeOf(fn));
}


static bool llvm_create_blockexit(const struct rir_block_exit *e, struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef b;
    LLVMBasicBlockRef other_b;
    LLVMValueRef cond;
    switch (e->type) {
    case RIR_BLOCK_EXIT_BRANCH:
        if (!(b = bllvm_value_from_rir_value(e->branch.dst, ctx))) {
            RF_ERROR("Failed to retrieve llvm block from values map");
            return false;
        }
        LLVMBuildBr(ctx->builder, b);
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        if (!(cond = bllvm_value_from_rir_value(e->condbranch.cond, ctx))) {
            RF_ERROR("Failed to retrieve llvm condition from value map");
            return false;
        }
        if (!(b = bllvm_value_from_rir_value(e->condbranch.taken, ctx))) {
            RF_ERROR("Failed to retrieve llvm taken block from values map");
            return false;
        }
        if (!(other_b = bllvm_value_from_rir_value(e->condbranch.fallthrough, ctx))) {
            RF_ERROR("Failed to retrieve llvm fallthrough block from values map");
            return false;
        }
        LLVMBuildCondBr(ctx->builder, cond, b, other_b);
        break;
    case RIR_BLOCK_EXIT_RETURN:
        if (e->retstmt.val) {
            LLVMBuildRet(ctx->builder, bllvm_value_from_rir_value_or_die(e->retstmt.val, ctx));
        } else {
            LLVMBuildRetVoid(ctx->builder);
        }
        break;
    default:
        RF_CRITICAL_FAIL("Invalid block exit type");
        return false;
    }
    return true;
}

static bool llvm_create_block(
    const struct rir_block *b,
    struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef llvmval;
    // create and enter the block
    RFS_PUSH();
    LLVMBasicBlockRef llvm_b = LLVMAppendBasicBlockInContext(
        ctx->llvm_context,
        ctx->current_function,
        rf_string_cstr_from_buff_or_die(rir_block_label_str(b))
    );
    RFS_POP();
    bllvm_enter_block(ctx, llvm_b);
    // also add the block to the map
    if(!llvm_traversal_ctx_map_llvmblock(ctx, &b->label, llvm_b)) {
        RF_ERROR("Failed to map a rir block to an llvm block");
        return false;
    }
    // if it's the first block of a function, alloca the return value.
    // In the RIR code it "just exists", just like the arguments allocas, so
    // we have to do it manually here
    if (rir_block_is_first(b) && ctx->current_rfn->retslot_val) {
        struct rir_type *rettype = ctx->current_rfn->retslot_val->type;
        // Val->type is the type of the alloca, so drop the pointer from it
        rettype = rir_type_get_or_create_from_other(
            rettype,
            llvm_traversal_ctx_rir(ctx),
            false
        );

        llvmval = LLVMBuildAlloca(
            ctx->builder,
            bllvm_type_from_rir_type(rettype, ctx),
            ""
        );
        if (!llvmval) {
            return false;
        }
        if (!llvm_traversal_ctx_map_llvmval(ctx, ctx->current_rfn->retslot_val, llvmval)) {
            RF_ERROR("Could not map return rir alloca to llvm alloca");
            return false;
        }
    }

    // now create llvm expressions out of the rir expressions of the block
    struct rir_expression *expr;
    rf_ilist_for_each(&b->expressions, expr, ln) {
        if (!(llvmval = bllvm_compile_rirexpr(expr, ctx))) {
            RF_ERROR(
                "Failed to compile rir expression \""RFS_PF"\"",
                RFS_PA(rir_expression_type_string(expr))
            );
            return false;
        }
    }
    return true;
}

static bool llvm_connect_block(const struct rir_block *b, struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef llvm_b = bllvm_value_from_rir_value_or_die(&b->label, ctx);
    bllvm_enter_block(ctx, llvm_b);
    return llvm_create_blockexit(&b->exit, ctx);
}

static bool bllvm_create_fndef(
    const struct rir_fndef *fn,
    struct llvm_traversal_ctx *ctx)
{
    struct rir_block **b;
    // create all blocks and their contents
    darray_foreach(b, fn->blocks) {
        if (!llvm_create_block(*b, ctx)) {
            return false;
        }
    }
    // and now that they are created connect them
    darray_foreach(b, fn->blocks) {
        if (!llvm_connect_block(*b, ctx)) {
            return false;
        }
    }
    return true;
}

static struct LLVMOpaqueValue *bllvm_create_fndecl(
    struct rir_fndecl *fn,
    struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef *arg_types = bllvm_rir_to_llvm_types(&fn->argument_types, ctx);
    // arg_types can also be null here, if the function has no arguments
    RFS_PUSH();
    LLVMValueRef llvmfn = LLVMAddFunction(
        ctx->llvm_mod,
        rf_string_cstr_from_buff_or_die(&fn->name),
        LLVMFunctionType(
            bllvm_type_from_rir_type(fn->return_type, ctx),
            arg_types,
            darray_size(fn->argument_types),
            false //no variable args for now
        ));
    RFS_POP();
    return llvmfn;
}

bool bllvm_create_module_functions(struct rir *r, struct llvm_traversal_ctx *ctx)
{
    struct rir_fndecl *decl;
    LLVMValueRef llvmfn;
    rf_ilist_for_each(&r->functions, decl, ln) {
        // before every function make sure the rir to llvm value map is clear
        llvm_traversal_ctx_reset_valmap(ctx);
        // create function declaration
        if (!(llvmfn = bllvm_create_fndecl(decl, ctx))) {
            RF_ERROR("Failed to create a function declaration in LLVM");
            return false;
        }

        // if it's also a function definition create a body for it
        if (!decl->plain_decl) {
            // make it the current function
            ctx->current_function = llvmfn;
            ctx->current_rfn = rir_fndecl_to_fndef(decl);
            if (!bllvm_create_fndef(ctx->current_rfn, ctx)) {
                RF_ERROR("Failed to create a function definition in LLVM");
                return false;
            }
        }
    }
    return true;
}
