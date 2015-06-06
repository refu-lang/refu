#include "llvm_matchexpr.h"

#include <llvm-c/Core.h>

#include "llvm_ast.h"
#include "llvm_utils.h"
#include "llvm_operators.h"

#include <ast/matchexpr.h>
#include <types/type.h>
#include <types/type_elementary.h>
#include <ir/rir_type.h>

static void bllvm_add_matchcase(struct ast_node *matchcase,
                                const struct rir_type *matching_type,
                                LLVMValueRef llvm_ret_alloca,
                                const struct type *ret_type,
                                LLVMValueRef llvm_typedecl_val,
                                LLVMValueRef llvm_switch,
                                LLVMBasicBlockRef match_end,
                                struct llvm_traversal_ctx *ctx)
{
    const struct type *case_matched_type = ast_matchcase_matched_type(matchcase);
    int index = rir_type_childof_type(type_get_rir_or_die(case_matched_type), matching_type);
    RF_ASSERT(index != -1, "Case type not found in matching type");
    LLVMBasicBlockRef case_branch = LLVMInsertBasicBlock(match_end, "");
    bllvm_enter_block(ctx, case_branch);
    // create backend handles (BuildAlloca) for the symbols of the symbol table
    symbol_table_iterate(ctx->current_st, (htable_iter_cb)llvm_symbols_iterate_cb, ctx);
    if (llvm_ret_alloca) {
        // if match does not return void
        // compile match case's expression and assign to type
        LLVMValueRef gep_to_main_contents = bllvm_gep_to_struct(llvm_typedecl_val, 0, ctx);
        bllvm_compile_assign_llvm(gep_to_main_contents, llvm_ret_alloca, ret_type, BLLVM_ASSIGN_MATCH_CASE, ctx);
    } else {
        bool first;
        struct symbol_table_record *rec = symbol_table_lookup_typedesc(
            ctx->current_st,
            ast_matchcase_pattern(matchcase),
            &first
        );
        RF_ASSERT(rec && rec->backend_handle && first,
                  "Symbol must be found in the first symbol table");
        
        bllvm_compile_assign_llvm(
            llvm_typedecl_val,
            rec->backend_handle,
            rec->data,
            BLLVM_ASSIGN_MATCH_CASE,
            ctx
        );
        // simply compile the matchcase expression which should return nothing
        bllvm_compile_expression(ast_matchcase_expression(matchcase), ctx, 0);
    }
    LLVMBuildBr(ctx->builder, match_end);
    LLVMAddCase(llvm_switch, LLVMConstInt(LLVMInt32Type(), index, 0), case_branch);
}

struct LLVMOpaqueValue *bllvm_compile_matchexpr(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx)
{
    struct ast_matchexpr_it it;
    struct ast_node *mcase;
    // find the type of the matched expression
    LLVMValueRef llvm_matched_value;
    const struct rir_type *matched_type_contents;
    if (ast_matchexpr_has_header(n)) {
        // if it's a normal match expression
        struct symbol_table_record *rec;
        const struct RFstring *matched_value_str = ast_matchexpr_matched_value_str(n);
        rec = symbol_table_lookup_record(ctx->current_st, matched_value_str, NULL);
        RF_ASSERT(rec && rec->data && rec->backend_handle,
                  "at backend: match expression type was not properly found"
                  "in the symbol table");
        RF_ASSERT(type_is_sumtype(rec->data),
                  "at backend: match expression type was not a sumtype");
        matched_type_contents = rir_type_contents(type_get_rir_or_die(rec->data));
        llvm_matched_value = rec->backend_handle;
    } else {
        // if it's a match expression acting as function body
        llvm_matched_value = LLVMGetParam(ctx->current_function, 0);
        matched_type_contents = type_get_rir_or_die(
            ast_node_get_type_or_nil(ast_matchexpr_headless_args(n), AST_TYPERETR_AS_LEAF)
        );
    }
    RF_ASSERT(llvm_matched_value, "Could not get llvm Value ref for the object");
    
    // allocate a return value if needed
    LLVMValueRef ret_alloc = NULL;
    if (!type_is_specific_elementary(ast_node_get_type(n, AST_TYPERETR_DEFAULT), ELEMENTARY_TYPE_NIL)) {
        LLVMTypeRef ret_llvm_type = bllvm_type_from_type(
            ast_node_get_type_or_nil(n, AST_TYPERETR_DEFAULT),
            ctx
        );
        ret_alloc = LLVMBuildAlloca(ctx->builder, ret_llvm_type, "");
    }

    LLVMBasicBlockRef match_end = bllvm_add_block_before_funcend(ctx);
    LLVMBasicBlockRef fatal_err_block = bllvm_add_fatal_block_before(match_end, 1, ctx);

    LLVMValueRef gep_to_selector = bllvm_gep_to_struct(llvm_matched_value, 1, ctx);
    LLVMValueRef selector_val = LLVMBuildLoad(ctx->builder, gep_to_selector, "");
    LLVMValueRef llvm_switch = LLVMBuildSwitch(
        ctx->builder,
        selector_val,
        fatal_err_block,
        ast_matchexpr_cases_num(n)
    );

    struct symbol_table *encasing_block_st = ctx->current_st;
    ast_matchexpr_foreach(n, &it, mcase) {
        // switch to the match case symbol table
        ctx->current_st = ast_matchcase_symbol_table_get(mcase);
        bllvm_add_matchcase(
            mcase,
            matched_type_contents,
            ret_alloc,
            ast_node_get_type(n, AST_TYPERETR_AS_LEAF),
            llvm_matched_value,
            llvm_switch,
            match_end,
            ctx
        );
    }

    ctx->current_st = encasing_block_st;
    bllvm_enter_block(ctx, match_end);
    return ret_alloc;
}
