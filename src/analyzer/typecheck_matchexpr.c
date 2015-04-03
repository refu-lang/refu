#include "typecheck_matchexpr.h"

#include <ast/ast.h>
#include <ast/matchexpr.h>
#include <analyzer/analyzer.h>

#include <types/type_comparisons.h>
#include <types/type.h>

enum traversal_cb_res typecheck_matchcase(struct ast_node *n, struct analyzer_traversal_ctx* ctx)
{
    struct ast_node *parent_matchexpr_id = ast_matchexpr_identifier(
        analyzer_traversal_ctx_get_nth_parent(0, ctx)
    );
    const struct type *case_pattern_type = ast_matchcase_pattern(n)->expression_type;
    const struct type *match_type = parent_matchexpr_id->expression_type;
    RF_ASSERT(case_pattern_type, "a type for the match case should have been determined");
    if (!type_compare(case_pattern_type, match_type, TYPECMP_PATTERN_MATCHING)) {
        RFS_buffer_push();
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "Match case \""RF_STR_PF_FMT"\" can not be matched to the "
                     "type of \""RF_STR_PF_FMT"\" which is of type \""RF_STR_PF_FMT"\".",
                     RF_STR_PF_ARG(type_str(case_pattern_type, false)),
                     RF_STR_PF_ARG(ast_identifier_str(parent_matchexpr_id)),
                     RF_STR_PF_ARG(type_str(match_type, true)));
        RFS_buffer_pop();
        return TRAVERSAL_CB_ERROR;
    }
    return TRAVERSAL_CB_OK;
}

enum traversal_cb_res typecheck_matchexpr(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx)
{
    (void)n;
    (void)ctx;
    // TODO: set the type of the match expression as a SUM type of all its cases
    return TRAVERSAL_CB_OK;
}
