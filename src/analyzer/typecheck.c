#include <analyzer/typecheck.h>

#include <Utils/build_assert.h>
#include <Utils/bits.h>
#include <Persistent/buffers.h>

#include <ast/ast.h>
#include <ast/operators.h>
#include <ast/function.h>
#include <ast/block.h>
#include <ast/constant_num.h>
#include <ast/vardecl.h>
#include <ast/ast_utils.h>
#include <ast/type.h>

#include <types/type.h>
#include <types/type_builtin.h>

#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include "analyzer_pass1.h" // for analyzer_make_parent_st_current()


static bool analyzer_typecheck_equal_or_convertible(struct ast_node *n,
                                                    enum binaryop_type operation,
                                                    const struct type *tleft,
                                                    const struct type *tright,
                                                    struct analyzer_traversal_ctx *ctx)
{
    struct type_comparison_ctx cmp_ctx;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);
    // comparison reason and operation share common enum values and as such this cast is valid
    type_comparison_ctx_init(&cmp_ctx, (enum comparison_reason)operation);

    if (type_equals(tleft, tright, &cmp_ctx)) {
        if (ctx->a->warn_on_implicit_conversions) {
            if (RF_BITFLAG_ON(cmp_ctx.conversion, SIGNED_TO_UNSIGNED)) {
                analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                              RF_STR_PF_FMT" from a signed to an unsigned type."
                              "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                              ast_binaryop_operation_name_str(operation),
                              RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                              RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            }

            if (RF_BITFLAG_ON(cmp_ctx.conversion, LARGER_TO_SMALLER)) {
                    analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                                  RF_STR_PF_FMT" from a larger to a smaller builtin type."
                                  "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                                  ast_binaryop_operation_name_str(operation),
                                  RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                                  RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            }
        }
        ret = true;
    }


    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}


static bool analyzer_types_addable(struct ast_node *left,
                                   struct ast_node *right,
                                   struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be added
    // if for example there is an addition typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static bool analyzer_typecheck_addition(struct ast_node *n,
                                        struct ast_node *left,
                                        struct ast_node *right,
                                        struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);

    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!analyzer_typecheck_equal_or_convertible(n, BINARYOP_ADD, tleft, tright, ctx)) {
        if (!analyzer_types_addable(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Addition between incompatible types. Can't add "
                         "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                         RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            goto end;
        }
    }

    // set the type of the addition as the type of either of its operands for now
    n->expression_type = tright;
    ret = true;

end:
    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}

static bool analyzer_types_subtractable(struct ast_node *left,
                                        struct ast_node *right,
                                        struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be subtracted
    // if for example there is a subtraction typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static bool analyzer_typecheck_subtraction(struct ast_node *n,
                                           struct ast_node *left,
                                           struct ast_node *right,
                                           struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);

    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!analyzer_typecheck_equal_or_convertible(n, BINARYOP_SUB, tleft, tright, ctx)) {
        if (!analyzer_types_subtractable(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Subtraction between incompatible types. Can't subtract "
                         "\""RF_STR_PF_FMT"\" from \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                         RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            goto end;
        }
    }

    // set the type of the addition as the type of either of its operands for now
    n->expression_type = tright;
    ret = true;

end:
    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}

static bool analyzer_types_multipliable(struct ast_node *left,
                                        struct ast_node *right,
                                        struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be multiplied
    // if for example there is a multiplication typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static bool analyzer_typecheck_multiplication(struct ast_node *n,
                                              struct ast_node *left,
                                              struct ast_node *right,
                                              struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);

    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!analyzer_typecheck_equal_or_convertible(n, BINARYOP_MUL, tleft, tright, ctx)) {
        if (!analyzer_types_multipliable(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Multiplication between incompatible types. Can't multiply "
                         "\""RF_STR_PF_FMT"\" by \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                         RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            goto end;
        }
    }

    // set the type of the addition as the type of either of its operands for now
    n->expression_type = tright;
    ret = true;

end:
    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}

static bool analyzer_types_divisible(struct ast_node *left,
                                     struct ast_node *right,
                                     struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be divided
    // if for example there is a division typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static bool analyzer_typecheck_division(struct ast_node *n,
                                        struct ast_node *left,
                                        struct ast_node *right,
                                        struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);

    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!analyzer_typecheck_equal_or_convertible(n, BINARYOP_DIV, tleft, tright, ctx)) {
        if (!analyzer_types_divisible(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Division between incompatible types. Can't divide "
                         "\""RF_STR_PF_FMT"\" by \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                         RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            goto end;
        }
    }

    // set the type of the addition as the type of either of its operands for now
    n->expression_type = tright;
    ret = true;

end:
    rf_buffer_set_index(TSBUFFA, buffer_index, char);
    return ret;
}

static bool analyzer_typecheck_constantnum(struct ast_node *n)
{
    n->expression_type = ast_constantnum_get_storagetype(n);
    return n->expression_type; //converted to bool
}

static bool analyzer_typecheck_identifier(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx)
{
    n->expression_type = type_lookup_identifier(n, ctx->current_st);

    if (!n->expression_type) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Type of identifier \""RF_STR_PF_FMT"\" is unknown",
                     RF_STR_PF_ARG(ast_identifier_str(n)));
        return false;
    }

    return true;
}

static bool analyzer_typecheck_xidentifier(struct ast_node *n,
                                           struct analyzer_traversal_ctx *ctx)
{
    (void)ctx;
    n->expression_type = n->xidentifier.id->expression_type;
    return true;
}

static bool analyzer_typecheck_typedesc(struct ast_node *n,
                                        struct analyzer_traversal_ctx *ctx)
{
    (void)ctx;
    n->expression_type = ast_typedesc_right(n)->expression_type;
    return true;
}


static bool analyzer_types_assignable(struct ast_node *left,
                                      struct ast_node *right,
                                      struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be assignable
    // if for example there is an assignment typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static bool analyzer_typecheck_assignment(struct ast_node *n,
                                          struct ast_node *left,
                                          struct ast_node *right,
                                          struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    uint32_t buffer_index;
    bool ret = false;

    buffer_index = rf_buffer_index(TSBUFFA);
    // left side of an assignment should be an identifier or a variable declaration
    if (left->type != AST_IDENTIFIER && left->type != AST_VARIABLE_DECLARATION) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier or a variable declaration "
                     "as left part of the assignment "
                     "but found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
        return false;
    }

    tright = ast_expression_get_type(right);
    tleft = ast_expression_get_type(left);

    if (!analyzer_typecheck_equal_or_convertible(n, BINARYOP_ASSIGN, tleft, tright, ctx)) {
        if (!analyzer_types_assignable(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Assignment between incompatible types. Can't assign "
                         "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright, TSBUFFA)),
                         RF_STR_PF_ARG(type_str(tleft, TSBUFFA)));
            goto end;
        }
    }

    // set the type of assignment as the type of the left operand
    n->expression_type = tleft;
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

    //TODO: more binary operators
    switch (ast_binaryop_op(n)) {
    case BINARYOP_ASSIGN:
        return analyzer_typecheck_assignment(n, left, right, ctx);
    case BINARYOP_ADD:
        return analyzer_typecheck_addition(n, left, right, ctx);
    case BINARYOP_SUB:
        return analyzer_typecheck_subtraction(n, left, right, ctx);
    case BINARYOP_MUL:
        return analyzer_typecheck_multiplication(n, left, right, ctx);
    case BINARYOP_DIV:
        return analyzer_typecheck_division(n, left, right, ctx);
    default:
        // nothing to do
        break;
    }

    return true;
}

static bool analyzer_typecheck_descend(struct ast_node *n, void *user_arg)
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

    default:
        // do nothing
        break;
    }

    return true;
}


static bool analyzer_typecheck_do(struct ast_node *n,
                                  void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = (struct analyzer_traversal_ctx*)user_arg;
    bool ret = true;

    // TODO: Temporary debug string commented out for quick check while testing, delete soon
    /* printf("Typechecking "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n))); */
    /* fflush(stdout); */

    switch(n->type) {
    case AST_BINARY_OPERATOR:
        ret = analyzer_typecheck_binary_op(n, ctx);
        break;
    case AST_IDENTIFIER:
        ret = analyzer_typecheck_identifier(n, ctx);
        break;
    case AST_XIDENTIFIER:
        ret = analyzer_typecheck_xidentifier(n, ctx);
        break;
    case AST_TYPE_DESCRIPTION:
        ret = analyzer_typecheck_typedesc(n, ctx);
        break;
    case AST_CONSTANT_NUMBER:
        ret = analyzer_typecheck_constantnum(n);
        break;
    case AST_STRING_LITERAL:
        n->expression_type = type_builtin_get_type(BUILTIN_STRING);
        ret = n->expression_type;
        break;
    case AST_VARIABLE_DECLARATION:
        // for a variable definition, the variable's type description should be
        // the type of the expression
        n->expression_type = ast_vardecl_desc_get(n)->expression_type;
        break;
    default:
        // do nothing. Think what to do for the remaining nodes if anything ...
        break;
    }

    // also change symbol table upwards if needed
    analyzer_make_parent_st_current(n, ctx);
    return ret;
}

bool analyzer_typecheck(struct analyzer *a)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, a);

    return ast_traverse_tree(
        a->root,
        analyzer_typecheck_descend,
        &ctx,
        analyzer_typecheck_do,
        &ctx);
}


