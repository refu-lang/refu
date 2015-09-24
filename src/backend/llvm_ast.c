#include "llvm_ast.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>
#include <llvm-c/Linker.h>

#include <Data_Structures/intrusive_list.h>
#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>
#include <Utils/sanity.h>
#include <Utils/bits.h>

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

#include <types/type_function.h>
#include <types/type_elementary.h>
#include <types/type_utils.h>
#include <types/type_comparisons.h>
#include <types/type.h>

#include <ir/rir_type.h>

#include <backend/llvm.h>
#include "llvm_utils.h"
#include "llvm_globals.h"
#include "llvm_operators.h"
#include "llvm_functions.h"
#include "llvm_types.h"
#include "llvm_matchexpr.h"

LLVMTypeRef bllvm_elementary_to_type(enum elementary_type etype,
                                     struct llvm_traversal_ctx *ctx)
{
    switch(etype) {
        // LLVM does not differentiate between signed and unsigned
    case ELEMENTARY_TYPE_INT_8:
    case ELEMENTARY_TYPE_UINT_8:
        return LLVMInt8Type();
    case ELEMENTARY_TYPE_INT_16:
    case ELEMENTARY_TYPE_UINT_16:
        return LLVMInt16Type();
    case ELEMENTARY_TYPE_INT_32:
    case ELEMENTARY_TYPE_UINT_32:
        return LLVMInt32Type();
    case ELEMENTARY_TYPE_INT:
    case ELEMENTARY_TYPE_UINT:
    case ELEMENTARY_TYPE_INT_64:
    case ELEMENTARY_TYPE_UINT_64:
        return LLVMInt64Type();

    case ELEMENTARY_TYPE_FLOAT_32:
        return LLVMFloatType();
    case ELEMENTARY_TYPE_FLOAT_64:
        return LLVMDoubleType();

    case ELEMENTARY_TYPE_STRING:
        return LLVMGetTypeByName(ctx->llvm_mod, "string");

    case ELEMENTARY_TYPE_BOOL:
        return LLVMInt1Type();
    case ELEMENTARY_TYPE_NIL:
        return LLVMVoidType();

    default:
        RF_CRITICAL_FAIL(
            "Unsupported elementary type \""RF_STR_PF_FMT"\" "
            "during LLVM conversion",
            RF_STR_PF_ARG(type_elementary_get_str(etype)));
        break;
    }
    return NULL;
}

LLVMTypeRef bllvm_type_from_type(const struct type *type,
                                 struct llvm_traversal_ctx *ctx)
{
    char *name;
    LLVMTypeRef ret = NULL;
    if (type->category == TYPE_CATEGORY_ELEMENTARY) {
        ret = bllvm_elementary_to_type(type_elementary(type), ctx);
    } else if (type->category == TYPE_CATEGORY_LEAF) {
        ret = bllvm_type_from_type(type->leaf.type, ctx);
    } else if (type->category == TYPE_CATEGORY_DEFINED) {
        RFS_PUSH();
        name = rf_string_cstr_from_buff_or_die(type_defined_get_name(type));
        ret = LLVMGetTypeByName(ctx->llvm_mod, name);
        RFS_POP();
    } else if (type_is_sumtype(type)) {
        RFS_PUSH();
        name = rf_string_cstr_from_buff_or_die(
            type_get_unique_type_str(type, false)
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

LLVMValueRef bllvm_cast_value_to_elementary_maybe(LLVMValueRef val,
                                                  const struct type *t,
                                                  struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_ELEMENTARY,
              "Casting only to elementary types supported for now");
    LLVMTypeRef common_type = bllvm_elementary_to_type(type_elementary(t), ctx);
    return bllvm_cast_value_to_type_maybe(val, common_type, ctx);
}

LLVMValueRef bllvm_compile_explicit_cast(const struct type *cast_type,
                                         struct ast_node *args,
                                         struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef cast_value = bllvm_compile_expression(
        args,
        ctx,
        RFLLVM_OPTION_IDENTIFIER_VALUE);
    // at the moment only cast to string requires special work
    if (cast_type->elementary.etype != ELEMENTARY_TYPE_STRING) {
        return bllvm_cast_value_to_elementary_maybe(cast_value, cast_type, ctx);
    }

    // from here and down it's a cast to string
    if (args->type == AST_CONSTANT) {
        RFS_PUSH();
        LLVMValueRef ret_str;
        struct RFstring *temps;
        switch (ast_constant_get_type(args)) {
        case CONSTANT_NUMBER_INTEGER:
            temps = RFS("%"PRIu64, args->constant.value.integer);
            break;
        case CONSTANT_NUMBER_FLOAT:
            // for now float conversion to string will use 4 decimal digits precision
            temps = RFS("%.4f", args->constant.value.floating);
            break;
        case CONSTANT_BOOLEAN:
            return args->constant.value.boolean
                ? bllvm_get_boolean_str(true, ctx)
                : bllvm_get_boolean_str(false, ctx);
        default:
            RF_CRITICAL_FAIL("Illegal constant number type encountered at code generation");
            break;
        }

        ret_str = bllvm_create_global_const_string(temps, ctx);
        RFS_POP();
        return ret_str;
    }

    if (type_is_specific_elementary(ast_node_get_type(args, AST_TYPERETR_DEFAULT), ELEMENTARY_TYPE_BOOL)) {
        LLVMBasicBlockRef taken_branch = bllvm_add_block_before_funcend(ctx);
        LLVMBasicBlockRef fallthrough_branch = bllvm_add_block_before_funcend(ctx);
        LLVMBasicBlockRef if_end = bllvm_add_block_before_funcend(ctx);
        LLVMValueRef string_alloca = LLVMBuildAlloca(ctx->builder, LLVMGetTypeByName(ctx->llvm_mod, "string"), "");
        LLVMBuildCondBr(ctx->builder, cast_value, taken_branch, fallthrough_branch);

        // if true
        bllvm_enter_block(ctx, taken_branch);
        bllvm_copy_string(bllvm_get_boolean_str(true, ctx),
                          string_alloca,
                          ctx);
        LLVMBuildBr(ctx->builder, if_end);
        // else false
        bllvm_enter_block(ctx, fallthrough_branch);
        bllvm_copy_string(bllvm_get_boolean_str(false, ctx),
                          string_alloca,
                          ctx);
        LLVMBuildBr(ctx->builder, if_end);
        bllvm_enter_block(ctx, if_end);
        return string_alloca;
    }

    // else
    RF_CRITICAL_FAIL("IIllegal cast, should not get here");
    return NULL;
}

LLVMValueRef bllvm_compile_vardecl(struct ast_node *n,
                                   struct llvm_traversal_ctx *ctx)
{
    // all vardelcs should have had stack size allocated during block symbol iteration
    struct symbol_table_record *rec;
    struct ast_node *left = ast_typeleaf_left((ast_vardecl_desc_get(n)));
    AST_NODE_ASSERT_TYPE(left, AST_IDENTIFIER);

    rec = symbol_table_lookup_record(ctx->current_st,
                                     ast_identifier_str(left), NULL);
    RF_ASSERT(rec->backend_handle, "No LLVMValue was determined for a vardecl");
    return rec->backend_handle;
}

LLVMValueRef bllvm_compile_string_literal(struct ast_node *n,
                                          struct llvm_traversal_ctx *ctx)
{
    // all unique string literals should have been declared as global strings
    RFS_PUSH();
    struct RFstring *temps = RFS_NT_OR_DIE("gstr_%u", ast_string_literal_get_hash(n));
    LLVMValueRef global_str = LLVMGetNamedGlobal(ctx->llvm_mod, rf_string_data(temps));
    RFS_POP();
    return global_str;
}

LLVMValueRef bllvm_compile_identifier(struct ast_node *n,
                                      struct llvm_traversal_ctx *ctx,
                                      int options)
{
    struct symbol_table_record *rec;
    const struct RFstring *s = ast_identifier_str(n);
    rec = symbol_table_lookup_record(ctx->current_st, s, NULL);
    RF_ASSERT(rec && rec->backend_handle, "No LLVMValue was determined for "
              "identifier \""RF_STR_PF_FMT"\"", RF_STR_PF_ARG(s));
    if (RF_BITFLAG_ON(options, RFLLVM_OPTION_IDENTIFIER_VALUE) &&
        ast_node_is_elementary_identifier(n)) {
        // then we actually need to load the value from memory
        return LLVMBuildLoad(ctx->builder, rec->backend_handle, "");
    }
    return rec->backend_handle;
}

void bllvm_compile_ifexpr(const struct ast_node *ifexpr,
                          struct llvm_traversal_ctx *ctx)
{
    LLVMBasicBlockRef taken_branch = bllvm_add_block_before_funcend(ctx);
    LLVMBasicBlockRef fallthrough_branch = bllvm_add_block_before_funcend(ctx);
    LLVMBasicBlockRef if_end = bllvm_add_block_before_funcend(ctx);

    // create the condition
    LLVMValueRef condition = bllvm_compile_expression(
        ast_ifexpr_condition_get(ifexpr),
        ctx,
        RFLLVM_OPTION_IDENTIFIER_VALUE);

    // Build the If conditional branch
    LLVMBuildCondBr(ctx->builder, condition, taken_branch, fallthrough_branch);

    // create the taken block
    bllvm_enter_block(ctx, taken_branch);
    bllvm_compile_block(ast_ifexpr_taken_block_get(ifexpr), ctx);    
    bllvm_add_br(if_end, ctx);

    // if there is a fall through block deal with it
    bllvm_enter_block(ctx, fallthrough_branch);
    struct ast_node *fallthrough = ast_ifexpr_fallthrough_branch_get(ifexpr);
    if (fallthrough) {
        bllvm_compile_branch(fallthrough, ctx);
    }
    bllvm_add_br(if_end, ctx);

    // enter the if end block
    bllvm_enter_block(ctx, if_end);
}

void bllvm_compile_branch(const struct ast_node *branch,
                          struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(branch->type == AST_IF_EXPRESSION || branch->type == AST_BLOCK,
              "Unexpected ast node type");
    if (branch->type == AST_IF_EXPRESSION) {
        bllvm_compile_ifexpr(branch, ctx);
    } else { // it's final else block
        bllvm_compile_block(branch, ctx);
    }
}

LLVMValueRef bllvm_compile_expression(struct ast_node *n,
                                      struct llvm_traversal_ctx *ctx,
                                      int options)
{
    int64_t int_val;
    double float_val;
    LLVMValueRef llvm_val;
    switch(n->type) {
    case AST_BINARY_OPERATOR:
        return bllvm_compile_bop(n, ctx);
    case AST_UNARY_OPERATOR:
        return bllvm_compile_uop(n, ctx);
    case AST_RETURN_STATEMENT:
        // assign the value to the function's return and jump to the final block
        llvm_val = bllvm_compile_expression(ast_returnstmt_expr_get(n),
                                            ctx,
                                            RFLLVM_OPTION_IDENTIFIER_VALUE);
        bllvm_compile_assign_llvm(llvm_val,
                                  ctx->current_function_return,
                                  ast_node_get_type(n, AST_TYPERETR_DEFAULT),
                                  BLLVM_ASSIGN_SIMPLE,
                                  ctx);
        LLVMBuildBr(ctx->builder, LLVMGetLastBasicBlock(ctx->current_function));
        break;
    case AST_FUNCTION_CALL:
        return bllvm_compile_functioncall(n, ctx);
    case AST_CONSTANT:
        switch (ast_constant_get_type(n)) {
        case CONSTANT_NUMBER_INTEGER:
            if (!ast_constant_get_integer(n, &int_val)) {
                RF_ERROR("Failed to convert a constant num node to integer number for LLVM");
            }
            ctx->current_value = (int_val >= 0)
                ? LLVMConstInt(LLVMInt32Type(), int_val, 0) 
                : LLVMConstNeg(LLVMConstInt(LLVMInt32Type(), (unsigned long long)-int_val, 0));
            break;
        case CONSTANT_NUMBER_FLOAT:
            if (!ast_constant_get_float(n, &float_val)) {
                RF_ERROR("Failed to convert a constant num node to float number for LLVM");
            }
            ctx->current_value = LLVMConstReal(LLVMDoubleType(), float_val);
            break;
        case CONSTANT_BOOLEAN:
            ctx->current_value = LLVMConstInt(LLVMInt1Type(), ast_constant_get_bool(n), 0);
            break;
        default:
            RF_CRITICAL_FAIL("Invalid constant type");
            break;
        }
        return ctx->current_value;
    case AST_STRING_LITERAL:
        return bllvm_compile_string_literal(n, ctx);
    case AST_IDENTIFIER:
        return bllvm_compile_identifier(n, ctx, options);
    case AST_VARIABLE_DECLARATION:
        return bllvm_compile_vardecl(n, ctx);
    case AST_TYPE_DECLARATION:
        RF_ASSERT(bllvm_compile_typedecl(ast_typedecl_name_str(n), NULL, ctx),
                  "typedecl compile should never fail");
        break;
    case AST_IF_EXPRESSION:
        bllvm_compile_ifexpr(n, ctx);
        break;
    case AST_MATCH_EXPRESSION:
        return bllvm_compile_matchexpr(n, ctx);
    default:
        RF_CRITICAL_FAIL("Illegal node type at LLVM code generation");
        break;
    }
    return NULL;
}

void llvm_symbols_iterate_cb(struct symbol_table_record *rec,
                             struct llvm_traversal_ctx *ctx)
{
    char *name;
    // for each symbol, allocate an LLVM variable in the stack with alloca
    struct type *type = symbol_table_record_type(rec);
    RFS_PUSH();
    name = rf_string_cstr_from_buff_or_die(symbol_table_record_id(rec));
    // note: this simply creates the stack space but does not allocate it
    LLVMTypeRef llvm_type = bllvm_type_from_type(type, ctx);
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, llvm_type, name);
    symbol_table_record_set_backend_handle(rec, allocation);
    RFS_POP();
}

void bllvm_compile_block(const struct ast_node *block,
                         struct llvm_traversal_ctx *ctx)
{
    struct ast_node *child;
    struct symbol_table *prev = ctx->current_st;
    ctx->current_st = ast_block_symbol_table_get((struct ast_node*)block);
    
    symbol_table_iterate(ctx->current_st, (htable_iter_cb)llvm_symbols_iterate_cb, ctx);
    rf_ilist_for_each(&block->children, child, lh) {
        bllvm_compile_expression(child, ctx, 0);
    }
    ctx->current_st = prev;
}

struct LLVMOpaqueModule *blvm_create_module(struct module *module,
                                            struct llvm_traversal_ctx *ctx,
                                            const struct RFstring *name,
                                            struct LLVMOpaqueModule *link_source)
{
    struct ast_node *child;
    // temporary. Name checking should be abstracted elsewhere
    const struct RFstring s_stdlib = RF_STRING_STATIC_INIT("stdlib");
    RFS_PUSH();
    const char *mod_name = rf_string_cstr_from_buff_or_die(module_name(module));
    if (!mod_name) {
        RF_ERROR("Failure to create null terminated cstring from RFstring");
        goto end;
    }
    ctx->llvm_mod = LLVMModuleCreateWithName(mod_name);
    ctx->target_data = LLVMCreateTargetData(LLVMGetDataLayout(ctx->llvm_mod));

    if (rf_string_equal(name, &s_stdlib)) {
        // create some global definitions that the stdlib should offer
        if (!bllvm_create_globals(ctx)) {
            RF_ERROR("Failed to create general globals for LLVM");
            LLVMDisposeModule(ctx->llvm_mod);
            goto end;
        }
    } else {
        char *error = NULL;
        RF_ASSERT(link_source, "If module is not stdlib, linking with something is mandatory at least for now");
        // if an error occurs LLVMLinkModules() returns true ...
        if (true == LLVMLinkModules(ctx->llvm_mod, link_source, LLVMLinkerDestroySource, &error)) {
            bllvm_error("Could not link LLVM modules", &error);
            goto end;
        }
        bllvm_error_dispose(&error);
    }
    if (!bllvm_create_module_globals(ctx)) {
        RF_ERROR("Failed to create module globals for LLVM");
            LLVMDisposeModule(ctx->llvm_mod);
            goto end;
    }

    // for each function of the module (for now simply the AST root) create code
    rf_ilist_for_each(&module->node->children, child, lh) {
        if (child->type == AST_FUNCTION_IMPLEMENTATION) {
            bllvm_compile_function(child, ctx);
        }
    }

    if (compiler_args_print_backend_debug(ctx->args)) {
        bllvm_mod_debug(ctx->llvm_mod, mod_name);
    }

end:
    RFS_POP();
    return ctx->llvm_mod;
}
