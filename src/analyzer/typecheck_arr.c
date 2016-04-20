#include <analyzer/typecheck_arr.h>

#include <rflib/string/core.h>
#include <rflib/string/conversion.h>

#include <ast/ast.h>
#include <ast/arr.h>
#include <ast/constants.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <types/type.h>
#include <types/type_arr.h>
#include <types/type_comparisons.h>

enum traversal_cb_res typecheck_bracketlist(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    struct ast_node **c;
    unsigned i = 1;

    struct arr_ast_nodes *members = ast_bracketlist_members(n);

    if (darray_size(*members) == 0) {
        // type of empty array
        n->expression_type = NULL;
        return TRAVERSAL_CB_OK;
    }
    struct ast_node *first_child = darray_item(*members, 0);
    const struct type *first_type = ast_node_get_type(first_child);
    if (!first_type) {
        analyzer_err(
            ctx->m,
            ast_node_startmark(first_child),
            ast_node_endmark(first_child),
            "Type of the first node in a bracket_list could not be determined"
        );
        return TRAVERSAL_CB_ERROR;
    }
    darray_foreach(c, *members) {
        if (!type_compare(first_type, ast_node_get_type(*c), TYPECMP_IMPLICIT_CONVERSION)) {
            RFS_PUSH();
            analyzer_err(
                ctx->m,
                ast_node_startmark(*c),
                ast_node_endmark(*c),
                "Type of the "RFS_PF" item in a bracket list is \""RFS_PF"\" "
                "which does not match the type of the first time \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(i)),
                RFS_PA(type_str_or_die(ast_node_get_type(*c), TSTR_DEFAULT)),
                RFS_PA(type_str_or_die(first_type, TSTR_DEFAULT))
            );
            RFS_POP();
            return TRAVERSAL_CB_ERROR;
        }
        i++;
    }

    // at the end create the array type
    traversal_node_set_type(
        n,
        module_getorcreate_type_as_singlearr(ctx->m, first_type, i - 1),
        ctx
    );
    return TRAVERSAL_CB_OK;
}

void typecheck_adjust_elementary_arr_const_values(
    struct ast_node *n,
    const struct type *tleft)
{
    RF_ASSERT(
        type_is_elementary_array(tleft),
        "Left type should also have been determined to be an array here"
    );
    RF_ASSERT(
        type_is_elementary_array(n->expression_type),
        "Bracket list should also be an elementary array here"
    );
    struct arr_ast_nodes *members = ast_bracketlist_members(n);
    if (darray_size(*members) == 0) {
        return;
    }

    if (type_get_elementary(type_array_member_type(n->expression_type)) ==
        type_get_elementary(type_array_member_type(tleft))) {

        return;
    }

    struct ast_node *first_child = darray_item(*members, 0);
    if (first_child->type != AST_CONSTANT) {
        return;
    }

    struct ast_node **c;
    darray_foreach(c, *members) {
        (*c)->expression_type = type_array_member_type(tleft);
    }
    n->expression_type = tleft;
    return;
}

enum traversal_cb_res typecheck_indexaccess(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *tleft;
    const struct type *tright;

    if (!(tleft = ast_node_get_type(left))) {
        analyzer_err(
            ctx->m,
            ast_node_startmark(left),
            ast_node_endmark(left),
            "Undeclared identifier \""RFS_PF"\" as left part of "
            "index access operator.",
            RFS_PA(ast_identifier_str(left))
        );
        return TRAVERSAL_CB_ERROR;
    }

    // left type of index access should be an array type
    if (tleft->category != TYPE_CATEGORY_ARRAY) {
        RFS_PUSH();
        analyzer_err(
            ctx->m,
            ast_node_startmark(left),
            ast_node_endmark(left),
            "Applying index access operator at non-array type \""RFS_PF"\".",
            RFS_PA(type_str_or_die(tleft, TSTR_DEFAULT))
        );
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }

    if (!(tright = ast_node_get_type(right))) {
        analyzer_err(
            ctx->m,
            ast_node_startmark(right),
            ast_node_endmark(right),
            "Could not determine the type of the index expression"
        );
        return TRAVERSAL_CB_ERROR;
    }

    if (!type_is_int_elementary(tright)) {
        RFS_PUSH();
        analyzer_err(
            ctx->m,
            ast_node_startmark(right),
            ast_node_endmark(right),
            "Expected an integer type for the index but got \""RFS_PF"\".",
            RFS_PA(type_str_or_die(tright, TSTR_DEFAULT))
        );
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }

    // we can have an extra check for constant indices of fixed size arrays
    // note: we completely ignore multidimensional arrays here
    int64_t arrsize;
    if (right->type == AST_CONSTANT && (arrsize = type_get_arr_first_size(tleft)) != -1) {
        int64_t val;
        RF_ASSERT(
            ast_constant_get_integer(&right->constant, &val),
            "Should never fail at this point"
        );
        if (val >= arrsize) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(right),
                ast_node_endmark(right),
                "Accessing array out of bounds. Array size is '%"PRIu64"' "
                "and you are attempting to access index '%"PRIu64"'.",
                arrsize, val
            );
            return TRAVERSAL_CB_ERROR;
        }
    }

    traversal_node_set_type(n, type_array_member_type(tleft), ctx);
    return TRAVERSAL_CB_OK;
}
