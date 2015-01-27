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
#include <ast/returnstmt.h>
#include <ast/type.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include "analyzer_pass1.h" // for analyzer symbol table change functions


static bool typecheck_equal_or_convertible(struct ast_node *n,
                                           enum binaryop_type operation,
                                           const struct type *tleft,
                                           const struct type *tright,
                                           struct analyzer_traversal_ctx *ctx)
{
    struct type_comparison_ctx cmp_ctx;
    bool ret = false;

    RFS_buffer_push();
    // comparison reason and operation share common enum values and as such this cast is valid
    type_comparison_ctx_init(&cmp_ctx, (enum comparison_reason)operation);

    if (type_equals(tleft, tright, &cmp_ctx)) {
        if (ctx->a->warn_on_implicit_conversions) {
            if (RF_BITFLAG_ON(cmp_ctx.conversion, SIGNED_TO_UNSIGNED)) {
                analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                              RF_STR_PF_FMT" from a signed to an unsigned type."
                              "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                              ast_binaryop_operation_name_str(operation),
                              RF_STR_PF_ARG(type_str(tright)),
                              RF_STR_PF_ARG(type_str(tleft)));
            }

            if (RF_BITFLAG_ON(cmp_ctx.conversion, LARGER_TO_SMALLER)) {
                analyzer_warn(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                              RF_STR_PF_FMT" from a larger to a smaller elementary type."
                              "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                              ast_binaryop_operation_name_str(operation),
                              RF_STR_PF_ARG(type_str(tright)),
                              RF_STR_PF_ARG(type_str(tleft)));
            }
        }
        ret = true;
    }

    RFS_buffer_pop();
    return ret;
}

static bool i_should_be_changed(struct ast_node *left,
                                struct ast_node *right,
                                struct analyzer_traversal_ctx *ctx)
{
    // TODO: this is just a callback which should never exist. Should check if op is applicable
    // Wherever it's used it's just temporary and should be changed to
    // either a callback per binary operator or something else
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

// Generic typecheck function for binary operations. If special functionality
// needs to be implemented for an operator do that in a separate function
static enum ast_traversal_cb_res typecheck_binaryop_generic(struct ast_node *n,
                                                            struct ast_node *left,
                                                            struct ast_node *right,
                                                            struct analyzer_traversal_ctx *ctx,
                                                            enum binaryop_type operation,
                                                            bool(*operator_applicable_cb)(struct ast_node*, struct ast_node*, struct analyzer_traversal_ctx*),
                                                            const char *error_intro,
                                                            const char *error_conj)
{
    const struct type *tright;
    const struct type *tleft;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_ERROR;

    RFS_buffer_push();
    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!typecheck_equal_or_convertible(n, operation, tleft, tright, ctx)) {
        if (!operator_applicable_cb(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "%s \""RF_STR_PF_FMT"\" %s \""RF_STR_PF_FMT"\"",
                         error_intro, RF_STR_PF_ARG(type_str(tright)),
                         error_conj, RF_STR_PF_ARG(type_str(tleft)));
            goto end;
        }
    }

    // set the type of the operation as the type of either of its operands
    n->expression_type = tright;
    ret = AST_TRAVERSAL_OK;
end:
    RFS_buffer_pop();
    return ret;
}

// Generic typecheck function for binary operations who type should be a boolean
static enum ast_traversal_cb_res typecheck_bool_binaryop_generic(struct ast_node *n,
                                                                 struct ast_node *left,
                                                                 struct ast_node *right,
                                                                 struct analyzer_traversal_ctx *ctx,
                                                                 enum binaryop_type operation,
                                                                 bool(*operator_applicable_cb)(struct ast_node*, struct ast_node*, struct analyzer_traversal_ctx*),
                                                                 const char *error_intro,
                                                                 const char *error_conj)
{
    const struct type *tright;
    const struct type *tleft;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_ERROR;

    RFS_buffer_push();
    tleft = ast_expression_get_type(left);
    tright = ast_expression_get_type(right);

    if (!typecheck_equal_or_convertible(n, operation, tleft, tright, ctx)) {
        if (!operator_applicable_cb(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "%s \""RF_STR_PF_FMT"\" %s \""RF_STR_PF_FMT"\"",
                         error_intro, RF_STR_PF_ARG(type_str(tright)),
                         error_conj, RF_STR_PF_ARG(type_str(tleft)));
            goto end;
        }
    }

    n->expression_type = type_elementary_get_type(ELEMENTARY_TYPE_BOOL);
    ret = AST_TRAVERSAL_OK;
end:
    RFS_buffer_pop();
    return ret;
}

static enum ast_traversal_cb_res typecheck_constantnum(struct ast_node *n)
{
    n->expression_type = ast_constantnum_get_storagetype(n);
    return (n->expression_type) ? AST_TRAVERSAL_OK : AST_TRAVERSAL_ERROR;
}

static enum ast_traversal_cb_res typecheck_identifier(struct ast_node *n,
                                                      struct analyzer_traversal_ctx *ctx)
{
    n->expression_type = type_lookup_identifier_string(ast_identifier_str(n),
                                                       ctx->current_st);

    if (!n->expression_type) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Type of identifier \""RF_STR_PF_FMT"\" is unknown",
                     RF_STR_PF_ARG(ast_identifier_str(n)));
        return AST_TRAVERSAL_ERROR;
    }

    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_xidentifier(struct ast_node *n,
                                                       struct analyzer_traversal_ctx *ctx)
{
    (void)ctx;
    n->expression_type = n->xidentifier.id->expression_type;
    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_typedesc(struct ast_node *n,
                                                    struct analyzer_traversal_ctx *ctx)
{
    (void)ctx;
    n->expression_type = ast_typedesc_right(n)->expression_type;
    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_typedecl(struct ast_node *n,
                                                    struct analyzer_traversal_ctx *ctx)
{
    (void)ctx;
    n->expression_type = ast_typedecl_typedesc_get(n)->expression_type;
    return AST_TRAVERSAL_OK;
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

static enum ast_traversal_cb_res typecheck_assignment(struct ast_node *n,
                                                      struct ast_node *left,
                                                      struct ast_node *right,
                                                      struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_OK;

    // left side of an assignment should be an identifier or a variable declaration
    if (left->type != AST_IDENTIFIER && left->type != AST_VARIABLE_DECLARATION) {
        analyzer_err(ctx->a, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Expected an identifier or a variable declaration "
                     "as left part of the assignment "
                     "but found a \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_node_str(left)));
        ret = AST_TRAVERSAL_ERROR;
    }

    RFS_buffer_push();
    tright = ast_expression_get_type(right);
    tleft = ast_expression_get_type(left);
    if (!typecheck_equal_or_convertible(n, BINARYOP_ASSIGN, tleft, tright, ctx)) {
        if (!analyzer_types_assignable(left, right, ctx)) {
            analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                         "Assignment between incompatible types. Can't assign "
                         "\""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\"",
                         RF_STR_PF_ARG(type_str(tright)),
                         RF_STR_PF_ARG(type_str(tleft)));
            ret = AST_TRAVERSAL_ERROR;
            goto end;
        }
    }

    // set the type of assignment as the type of the left operand
    n->expression_type = tleft;

end:
    RFS_buffer_pop();
    return ret;
}

static enum ast_traversal_cb_res typecheck_comma(struct ast_node *n,
                                                 struct ast_node *left,
                                                 struct ast_node *right,
                                                 struct analyzer_traversal_ctx *ctx)
{
    struct type *tright;
    struct type *tleft;

    tleft = (struct type*)ast_expression_get_type(left);
    tright = (struct type*)ast_expression_get_type(right);

    // for now at least let's assume that all types can have the comma
    // operator applied to them

    // set the type of assignment as the type of the left operand
    n->expression_type = type_create_from_operation(TYPEOP_PRODUCT, tleft, tright, ctx->a);
    if (!n->expression_type) {
        RF_ERROR("Could not create a type as a product of 2 other types.");
        return AST_TRAVERSAL_FATAL_ERROR;
    }

    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_function_call(struct ast_node *n,
                                                         struct analyzer_traversal_ctx *ctx)
{
    const struct RFstring *fn_name;
    const struct type *fn_type;
    const struct type *fn_declared_args_type;
    const struct type *fn_found_args_type;
    struct type_comparison_ctx cmp_ctx;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_OK;

    // check for existence of function
    fn_name = ast_fncall_name(n);
    fn_type = type_lookup_identifier_string(fn_name, ctx->current_st);
    if (!fn_type || !type_is_function((fn_type))) {
        analyzer_err(ctx->a, ast_node_startmark(n),
                     ast_node_endmark(n),
                     "Undefined function call \""RF_STR_PF_FMT"\" detected",
                     RF_STR_PF_ARG(fn_name));
        return AST_TRAVERSAL_ERROR;
    }

    //check that the types of its arguments do indeed match
    fn_declared_args_type = type_function_get_argtype(fn_type);
    fn_found_args_type = ast_expression_get_type(ast_fncall_args(n));
    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_FUNCTION_CALL);
    if (!type_equals(fn_declared_args_type, fn_found_args_type, &cmp_ctx)) {
        RFS_buffer_push();
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "function "RF_STR_PF_FMT"() is called with argument type of "
                     "\""RF_STR_PF_FMT"\" which does not match the expected "
                     "type of \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(fn_name),
                     RF_STR_PF_ARG(type_str(fn_found_args_type)),
                     RF_STR_PF_ARG(type_str(fn_declared_args_type)));
        RFS_buffer_pop();
        ret = AST_TRAVERSAL_ERROR;
    }

    n->expression_type = type_function_get_rettype(fn_type);
    return ret;
}

static enum ast_traversal_cb_res typecheck_return_stmt(struct ast_node *n,
                                                       struct analyzer_traversal_ctx *ctx)
{
    struct type_comparison_ctx cmp_ctx;
    struct ast_node *fn_decl = symbol_table_get_fndecl(ctx->current_st);
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_OK;
    const struct type *fn_type;
    if (!fn_decl) {
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "Return statement outside of function body");
        return AST_TRAVERSAL_ERROR;
    }

    // at this stage the function declaration won't have been typechecked.
    // Do it now .. which is not optimal since it will happen again when going upwards
    // TODO: perhaps add a check in the typecheck reverse tree traversal to not visit
    // certain subtrees if they are already typechecked.
    if (!analyzer_typecheck(ctx->a, fn_decl)) {
        return AST_TRAVERSAL_ERROR;
    }

    fn_type = ast_expression_get_type(fn_decl);
    const struct type *fn_ret_type = type_function_get_rettype(fn_type);
    const struct type *found_ret_type = ast_expression_get_type(ast_returnstmt_expr_get(n));

    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_GENERIC);
    if (!type_equals(fn_ret_type, found_ret_type, &cmp_ctx)) {
        RFS_buffer_push();
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "Return statement type \""RF_STR_PF_FMT"\" does not match "
                     "the expected return type of \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(type_str(found_ret_type)),
                     RF_STR_PF_ARG(type_str(fn_ret_type)));
        RFS_buffer_pop();
        ret = AST_TRAVERSAL_ERROR;
    }
    n->expression_type = found_ret_type;
    return ret;
}

static enum ast_traversal_cb_res typecheck_block(struct ast_node *n,
                                                 struct analyzer_traversal_ctx *ctx)
{
    // TODO: Here somehow find the type of the block, it should all depend on its
    // last expression/expression/statement or it can also be a sum product if it
    // has conditional exits from the block
    n->expression_type = NULL;

    
    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_binary_op(struct ast_node *n,
                                                     struct analyzer_traversal_ctx *ctx)
{
    struct ast_node *left = ast_binaryop_left(n);
    struct ast_node *right = ast_binaryop_right(n);
    enum binaryop_type bop_type = ast_binaryop_op(n);
    //TODO: more binary operators
    switch (bop_type) {
    case BINARYOP_ASSIGN:
        return typecheck_assignment(n, left, right, ctx);
    case BINARYOP_COMMA:
        return typecheck_comma(n, left, right, ctx);

    case BINARYOP_ADD:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't add", "to");
    case BINARYOP_SUB:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't subtract", "from");
    case BINARYOP_MUL:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't multiply", "by");
    case BINARYOP_DIV:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't divide", "by");

    case BINARYOP_CMP_EQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is equal to");
    case BINARYOP_CMP_NEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is not equal to");
    case BINARYOP_CMP_GT:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is greater than");
    case BINARYOP_CMP_GTEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is greater than or equal");
    case BINARYOP_CMP_LT:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is less than");
    case BINARYOP_CMP_LTEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is less than or equal");

    case BINARYOP_LOGIC_AND:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply logic operator && between", "and");
    case BINARYOP_LOGIC_OR:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply logic operator || between", "and");

    case BINARYOP_BITWISE_OR:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise OR to", "and");
    case BINARYOP_BITWISE_AND:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise AND to", "and");
    case BINARYOP_BITWISE_XOR:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise XOR to", "and");

    default:
        RF_ASSERT(false, "Typechecking for unimplemented binary "
                  "operator "RF_STR_PF_FMT,
                  RF_STR_PF_ARG(ast_binaryop_opstr(n)));
        return AST_TRAVERSAL_FATAL_ERROR;
    }

    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_fndecl(struct ast_node *n,
                                                  struct analyzer_traversal_ctx *ctx)
{
    struct type *t;
    t = type_lookup_identifier_string(ast_fndecl_name_str(n), ctx->current_st);
    if (!t) {
        RF_ERROR("Function declaration name not found in the symbol table at impossible point");
        return AST_TRAVERSAL_ERROR;
    }
    n->expression_type = t;
    return AST_TRAVERSAL_OK;
}

static enum ast_traversal_cb_res typecheck_do(struct ast_node *n,
                                              void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = (struct analyzer_traversal_ctx*)user_arg;
    enum ast_traversal_cb_res ret = AST_TRAVERSAL_OK;
    switch(n->type) {
    case AST_BINARY_OPERATOR:
        ret = typecheck_binary_op(n, ctx);
        break;
    case AST_IDENTIFIER:
        ret = typecheck_identifier(n, ctx);
        break;
    case AST_XIDENTIFIER:
        ret = typecheck_xidentifier(n, ctx);
        break;
    case AST_TYPE_DECLARATION:
        ret = typecheck_typedecl(n, ctx);
        break;
    case AST_TYPE_DESCRIPTION:
        ret = typecheck_typedesc(n, ctx);
        break;
    case AST_CONSTANT_NUMBER:
        ret = typecheck_constantnum(n);
        break;
    case AST_STRING_LITERAL:
        n->expression_type = type_elementary_get_type(ELEMENTARY_TYPE_STRING);
        break;
    case AST_VARIABLE_DECLARATION:
        // for a variable definition, the variable's type description should be
        // the type of the expression
        n->expression_type = ast_vardecl_desc_get(n)->expression_type;
        break;
    case AST_FUNCTION_CALL:
        ret = typecheck_function_call(n, ctx);
        break;
    case AST_FUNCTION_DECLARATION:
        ret = typecheck_fndecl(n, ctx);
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        n->expression_type = ast_expression_get_type(ast_fnimpl_fndecl_get(n));
        break;
    case AST_RETURN_STATEMENT:
        ret = typecheck_return_stmt(n, ctx);
        break;
    case AST_BLOCK:
        ret = typecheck_block(n, ctx);
        break;
    default:
        // do nothing. Think what to do for the remaining nodes if anything ...
        break;
    }

    // also change symbol table upwards if needed
    analyzer_handle_symbol_table_ascending(n, ctx);
    return ret;
}

bool analyzer_typecheck(struct analyzer *a, struct ast_node *root)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, a);

    return AST_TRAVERSAL_OK == ast_traverse_tree_nostop_post_cb(
        root,
        (ast_node_cb)analyzer_handle_symbol_table_descending,
        &ctx,
        typecheck_do,
        &ctx);
}


