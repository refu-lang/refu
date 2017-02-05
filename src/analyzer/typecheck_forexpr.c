#include <analyzer/typecheck_forexpr.h>

#include <ast/ast.h>
#include <ast/forexpr.h>
#include <ast/iterable.h>
#include <ast/identifier.h>

#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <types/type.h>
#include <types/type_arr.h>

bool typecheck_forexpr_descending(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    struct ast_node *iterable = ast_forexpr_iterable_get(n);
    struct ast_node *loopvar = ast_forexpr_loopvar_get(n);
    const struct type *titerable;

    // Essentially doing type inference on the loop variable based
    // on the type of the loop iterable
    switch (ast_iterable_type_get(iterable)) {
    case ITERABLE_COLLECTION:
    {
        // find the type of the loop iterable
        struct ast_node *iterable_id = ast_iterable_identifier_get(iterable);
        titerable = type_lookup_identifier_string(
            ast_identifier_str(iterable_id),
            ctx->current_st
        );
        if (!(titerable)) {
            analyzer_err(
                ctx->m, ast_node_startmark(iterable),
                ast_node_endmark(iterable),
                "Undeclared identifier \""RFS_PF"\" as the iterable of "
                "a for expression",
                RFS_PA(ast_identifier_str(iterable_id))
            );
            return false;
        }
        // the iterable should be an array type
        if (titerable->category != TYPE_CATEGORY_ARRAY) {
            RFS_PUSH();
            analyzer_err(
                ctx->m,
                ast_node_startmark(iterable),
                ast_node_endmark(iterable),
                "Trying to iterate over non-iterable type \""RFS_PF"\".",
                RFS_PA(type_str_or_die(titerable, TSTR_DEFAULT))
            );
            RFS_POP();
            return false;
        }
        // the type of the loop variable should be the member type of the array
        titerable = type_array_member_type(titerable);
    }
    break;
    case ITERABLE_RANGE:
        // the loop variable should be an int type
        titerable = type_elementary_get_type(ELEMENTARY_TYPE_INT_64);
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Should never get here");
        break;
    }

    //set loop variable type
    traversal_node_set_type(loopvar, titerable, ctx);
    // set current symbol table to the table of the for expression
    ctx->current_st = ast_forexpr_symbol_table_get(n);
    // and add the loop variable to the symbol table
    const struct RFstring *name = ast_identifier_str(loopvar);
    if (!symbol_table_add_node(ctx->current_st, ctx->m, name, loopvar)) {
        RF_ERROR("Could not add loop variable to for expression symbol table");
        return false;
    }

    return true;
}

enum traversal_cb_res typecheck_forexpr_ascending(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // forexpr type is essentially the returning type of the block
    const struct type *block_type = ast_node_get_type_or_die(ast_forexpr_body_get(n));
    const struct ast_node *iterable = ast_forexpr_iterable_get(n);
    const struct type *iterable_type = ast_node_get_type(iterable);
    struct type *forexpr_type;

    switch (ast_iterable_type_get(iterable)) {
    case ITERABLE_COLLECTION:
        forexpr_type = module_getorcreate_type_as_singlearr(
            ctx->m,
            block_type,
            // if the iterable type has finite size, the return type would have
            // the same size
            type_get_arr_first_size(iterable_type)
        );
        break;
    case ITERABLE_RANGE:
        forexpr_type = module_getorcreate_type_as_singlearr(
            ctx->m,
            type_elementary_get_type(ELEMENTARY_TYPE_INT_64),
            ast_iterable_range_number_of_loops(iterable)
        );
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Should never get here");
        break;
    }

    traversal_node_set_type(
        n,
        forexpr_type,
        ctx
    );
    return TRAVERSAL_CB_OK;
}

enum traversal_cb_res typecheck_iterable(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_ITERABLE);

    switch(n->iterable.type) {
    case ITERABLE_COLLECTION:
        traversal_node_set_type(n, ast_node_get_type(n->iterable.identifier), ctx);
        break;
    case ITERABLE_RANGE:
    {
        int64_t start = ast_iterable_range_start_get(n);
        int64_t step = ast_iterable_range_step_get(n);
        int64_t end = ast_iterable_range_end_get(n);

        if (step == 0) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Providing zero as range step is invalid."
            );
            return TRAVERSAL_CB_ERROR;
        }

        if (end <= start && step > 0) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Provided range start \"%"PRId64"\" is greater or equal to the"
                " end \"%"PRId64"\" while the step is increasing.",
                start,
                end
            );
            return TRAVERSAL_CB_ERROR;
        }

        if (start <= end && step < 0) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Provided range start \"%"PRId64"\" is less or equal to the"
                " end \"%"PRId64"\" while the step is decreasing.",
                start,
                end
            );
            return TRAVERSAL_CB_ERROR;
        }
        traversal_node_set_type(n, type_elementary_get_type(ELEMENTARY_TYPE_INT_64), ctx);
        break;
    }
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Should never get here");
        break;
    }
    return TRAVERSAL_CB_OK;
}
