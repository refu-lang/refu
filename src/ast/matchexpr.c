#include <ast/matchexpr.h>

struct ast_node *ast_matchcase_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *pattern,
                                      struct ast_node *expression)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_MATCH_CASE, start, end);
    if (!ret) {
        return NULL;
    }
   
    ast_node_register_child(ret, pattern, matchcase.pattern);
    ast_node_register_child(ret, expression, matchcase.expression);

    return ret;
}

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_MATCH_CASE, start, end);
    if (!ret) {
        return NULL;
    }
    ast_node_register_child(ret, id, matchexpr.identifier);
    return ret;
}

i_INLINE_INS bool ast_matchexpr_is_bodyless(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_matchexpr_identifier(const struct ast_node *n);
