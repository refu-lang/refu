#include <analyzer/typecheck.h>

#include <Utils/build_assert.h>
#include <Utils/bits.h>
#include <Persistent/buffers.h>

#include <ast/ast.h>
#include <ast/operators.h>
#include <ast/function.h>
#include <ast/block.h>
#include <ast/constant_num.h>
#include <ast/ast_utils.h>


#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include "analyzer_pass1.h" // for analyzer_make_parent_st_current()


static bool analyzer_typecheck_assignment(struct ast_node *n,
                                          struct ast_node *left,
                                          struct ast_node *right,
                                          struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    struct symbol_table_record *rec;
    struct type_comparison_ctx cmp_ctx;
    uint32_t buffer_index;
    bool at_first;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);
    // left side of an assignment should be an identifier (?)
    if (left->type != AST_IDENTIFIER) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier as left part of the assignment "
                     "but found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
        return false;
    }

    rec = symbol_table_lookup_record(ctx->current_st,
                                     ast_identifier_str(left),
                                     &at_first);

    if (!rec) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Type of identifier \""RF_STR_PF_FMT"\" is unknown",
                     RF_STR_PF_ARG(ast_identifier_str(left)));
        return false;
    }

    tright = expression_determine_type(right);
    tleft = symbol_table_record_type(rec);

    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_ASSIGNMENT);
    if (!type_equals(tleft, tright, &cmp_ctx)) {
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "Assignment between incompatible types. Can't assign "
                     "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                     RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
        goto end;
    }

    if (ctx->a->warn_on_implicit_conversions) {

        if (RF_BITFLAG_ON(cmp_ctx.conversion, SIGNED_TO_UNSIGNED)) {
            analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                          "Assignment from a signed to an unsigned type."
                          "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                          RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                          RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
        }

        if (RF_BITFLAG_ON(cmp_ctx.conversion, LARGER_TO_SMALLER)) {
            analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                          "Assignment from a larger to a smaller builtin type."
                          "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                          RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                          RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
        }
    }


    ret = true;
  end:
    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}
static bool analyzer_typecheck_binary_op(struct ast_node *n,
                                         struct analyzer_traversal_ctx *ctx)
{
    struct ast_node *left = ast_binaryop_left(n);
    struct ast_node *right = ast_binaryop_right(n);

    switch (ast_binaryop_op(n)) {
    case BINARYOP_ASSIGN:
        return analyzer_typecheck_assignment(n, left, right, ctx);
    default:
        // nothing to do
        break;
    }

    return true;
}

static bool analyzer_typecheck_do(struct ast_node *n,
                                   void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = (struct analyzer_traversal_ctx*)user_arg;
    switch(n->type) {
        // nodes that change the current symbol table
    case AST_ROOT:
        ctx->current_st = ast_root_symbol_table_get(n);
        break;
    case AST_BLOCK:
        ctx->current_st = ast_block_symbol_table_get(n);
        break;
    case AST_FUNCTION_DECLARATION:
        ctx->current_st = ast_fndecl_symbol_table_get(n);
        break;

        // nodes for which we actually need to typecheck
    case AST_BINARY_OPERATOR:
        return analyzer_typecheck_binary_op(n, ctx);
    default:
        // do nothing
        break;
    }

    return true;
}

bool analyzer_typecheck(struct analyzer *a)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, a);

    return ast_traverse_tree(
        a->root,
        analyzer_typecheck_do,
        &ctx,
        (ast_node_cb)analyzer_make_parent_st_current,
        &ctx);
}


/* -- functions having to do with determining the type of an expression */

static inline const struct type *expression_constantnum_get_type(struct ast_node *n)
{
    enum constant_type ctype = ast_constantnum_get_type(n);
    if (ctype == CONSTANT_NUMBER_INTEGER) {
        return type_builtin_get_type(BUILTIN_UINT_64);
    } else if (ctype == CONSTANT_NUMBER_FLOAT) {
        return type_builtin_get_type(BUILTIN_FLOAT_64);
    }

    // should never happen
    RF_ASSERT_OR_CRITICAL(false,
                          "Illegal constantnum type");
    return NULL;
}

const struct type *expression_determine_type(struct ast_node *expr)
{
    switch (expr->type) {
    case AST_CONSTANT_NUMBER:
        return expression_constantnum_get_type(expr);
        break;
    case AST_STRING_LITERAL:
        return type_builtin_get_type(BUILTIN_STRING);
    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Illegal ast node type \""RF_STR_PF_FMT"\" detected"
                              " in determining the type of an expression",
                              RF_STR_PF_ARG(ast_node_str(expr)));
        break;
    }
    return NULL;
}