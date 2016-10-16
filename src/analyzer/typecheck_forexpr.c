#include <analyzer/typecheck_forexpr.h>

#include <ast/ast.h>
#include <ast/forexpr.h>
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

    // find the type of the loop iterable
    titerable = type_lookup_identifier_string(ast_identifier_str(iterable), ctx->current_st);
    if (!(titerable)) {
        analyzer_err(ctx->m, ast_node_startmark(iterable),
                     ast_node_endmark(iterable),
                     "Undeclared identifier \""RFS_PF"\" as the iterable of "
                     "a for expression",
                     RFS_PA(ast_identifier_str(iterable)));
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
    // according to that set the type of the loop variable
    traversal_node_set_type(loopvar, type_array_member_type(titerable), ctx);

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
    traversal_node_set_type(
        n,
        ast_node_get_type_or_die(n->forexpr.body),
        ctx
    );
    return TRAVERSAL_CB_OK;
}
