#include <analyzer/typecheck_arr.h>

#include <rflib/string/core.h>
#include <rflib/string/conversion.h>

#include <ast/ast.h>
#include <ast/arr.h>
#include <analyzer/analyzer.h>
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
    n->expression_type = module_getorcreate_type_as_singlearr(ctx->m, first_type, i - 1);
    return TRAVERSAL_CB_OK;
}

void typecheck_adjust_elementary_arr_const_values(
    struct ast_node *n,    
    const struct type *tleft)
{
    RF_ASSERT(
        tleft->category == TYPE_CATEGORY_ELEMENTARY &&
        tleft->array,
        "Left type should also have been determined to be an array here"
    );
    RF_ASSERT(
        n->expression_type->category == TYPE_CATEGORY_ELEMENTARY &&
        n->expression_type->array,
        "Bracket list should also be an elementary array here"
    );
    struct arr_ast_nodes *members = ast_bracketlist_members(n);
    if (darray_size(*members) == 0) {
        return;
    }

    if (n->expression_type->elementary.etype == tleft->elementary.etype) {
        return;
    }

    struct ast_node *first_child = darray_item(*members, 0);
    if (first_child->type != AST_CONSTANT) {
        return;
    }
        
    struct ast_node **c;
    darray_foreach(c, *members) {
        (*c)->expression_type = type_elementary_get_type(tleft->elementary.etype);
    }
    n->expression_type = tleft;
    return;
}
