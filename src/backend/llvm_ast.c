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
#include <ast/operators.h>

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

    case ELEMENTARY_TYPE_NIL:
        return LLVMVoidType();
        break;

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

static LLVMValueRef backend_llvm_expression_compile_bop(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BINARY_OPERATOR);
    LLVMValueRef left = backend_llvm_expression_compile(ast_binaryop_left(n), ctx);
    LLVMValueRef right = backend_llvm_expression_compile(ast_binaryop_right(n), ctx);
    switch(ast_binaryop_op(n)) {
        // arithmetic
        // note: There are different LLVMBuild Arithmetic functions. See if some should
        //       be used in special situations
    case BINARYOP_ADD:
        return LLVMBuildAdd(ctx->builder, left, right, "left + right");
    case BINARYOP_SUB:
        return LLVMBuildSub(ctx->builder, left, right, "left - right");
    case BINARYOP_MUL:
        return LLVMBuildMul(ctx->builder, left, right, "left * right");
    case BINARYOP_DIV:
        return LLVMBuildUDiv(ctx->builder, left, right, "left / right");
    default:
        RF_ASSERT(false, "Illegal binary operation type at LLVM code generation");
        break;
    }
    return NULL;
}

LLVMValueRef backend_llvm_function_call_compile(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx)
{
    (void)n;
    (void)ctx;
    //TODO
    return NULL;
}

LLVMValueRef backend_llvm_expression_compile(struct ast_node *n,
                                             struct llvm_traversal_ctx *ctx)
{
    uint64_t val;
    LLVMValueRef llvm_val;
    switch(n->type) {
    case AST_BINARY_OPERATOR:
        return backend_llvm_expression_compile_bop(n, ctx);
    case AST_RETURN_STATEMENT:
        llvm_val = backend_llvm_expression_compile(ast_returnstmt_expr_get(n), ctx);
        return LLVMBuildRet(ctx->builder, llvm_val);
    case AST_FUNCTION_CALL:
        return backend_llvm_function_call_compile(n, ctx);
    case AST_CONSTANT_NUMBER:
        if (!ast_constantnum_get_integer(n, &val)) {
            RF_ERROR("Failed to convert a constant num node to number for LLVM");
        }
        // TODO: This is not using rir_types ... also maybe get rid of ctx->current_value?
        //       if we are going to be returning it anyway?
        ctx->current_value = LLVMConstInt(LLVMInt32Type(), val, 0);
        return ctx->current_value;
    default:
        RF_ASSERT(false, "Illegal node type at LLVM code generation");
        break;
    }
    return NULL;
}

static void backend_llvm_expression(struct ast_node *n,
                                    struct llvm_traversal_ctx *ctx)
{
    ctx->current_value = NULL;
    backend_llvm_expression_compile(n, ctx);
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
    RFS_buffer_push();
    fn_name = rf_string_cstr_from_buff(&fn->name);
    llvm_fn = LLVMAddFunction(ctx->mod, fn_name,
                              LLVMFunctionType(backend_llvm_type(fn->ret_type, ctx),
                                               backend_llvm_types(fn->arg_type, ctx),
                                               llvm_traversal_ctx_get_param_count(ctx),
                                               false)); // never variadic for now
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
        rec = symbol_table_lookup_record(fn->symbols, param_name_str, NULL);
        RF_ASSERT_OR_CRITICAL(rec, "Symbol table of rir_function did not contain expected parameter");
        symbol_table_record_set_backend_handle(rec, allocation);
    }

    // now handle function entry block
    backend_llvm_basic_block(fn->entry, ctx);
    return llvm_fn;
}

static bool backend_llvm_create_globals(struct llvm_traversal_ctx *ctx)
{
    // TODO Put all global/special functions here. e.g. print() e.t.c.
    (void)ctx;
    return true;
}

struct LLVMOpaqueModule *backend_llvm_create_module(struct rir_module *mod,
                                                    struct llvm_traversal_ctx *ctx)
{
    struct rir_function *fn;
    const char *mod_name;
    RFS_buffer_push();
    mod_name = rf_string_cstr_from_buff(&mod->name);
    ctx->mod = LLVMModuleCreateWithName(mod_name);
    if (!backend_llvm_create_globals(ctx)) {
        RF_ERROR("Failed to create global context for LLVM");
        return NULL;
    }
    RFS_buffer_pop();
    // for each function of the module create code
    rf_ilist_for_each(&mod->functions, fn, ln_for_module) {
        backend_llvm_function(fn, ctx);
    }
    return ctx->mod;
}
