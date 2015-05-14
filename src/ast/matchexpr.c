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
    ret->matchcase.matched_type = NULL;

    return ret;
}

i_INLINE_INS struct ast_node *ast_matchcase_pattern(const struct ast_node *n);
i_INLINE_INS const struct type *ast_matchcase_matched_type(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_matchcase_expression(const struct ast_node *n);
i_INLINE_INS struct symbol_table *ast_matchcase_symbol_table_get(const struct ast_node *n);
i_INLINE_INS void *ast_matchcase_symbol_table_set(struct ast_node *n, struct symbol_table *st);

struct ast_node *ast_matchexpr_create(const struct inplocation_mark *start,
                                      const struct inplocation_mark *end,
                                      struct ast_node *id)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_MATCH_EXPRESSION, start, end);
    if (!ret) {
        return NULL;
    }
    ret->matchexpr.match_cases_num = 0;
    if (id) {
        ast_node_register_child(ret, id, matchexpr.identifier);
    } else {
        ret->matchexpr.identifier = NULL;
    }
    return ret;
}

i_INLINE_INS size_t ast_matchexpr_cases_num(const struct ast_node *n);
i_INLINE_INS bool ast_matchexpr_is_bodyless(const struct ast_node *n);
i_INLINE_INS struct ast_node *ast_matchexpr_identifier(const struct ast_node *n);

void ast_matchexpr_add_case(struct ast_node *n, struct ast_node *mcase)
{
    ast_node_add_child(n, mcase);
    ++n->matchexpr.match_cases_num;
}

struct ast_node *ast_matchexpr_first_case(const struct ast_node *n,
                                          struct ast_matchexpr_it *it)
{
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    unsigned int i = 0;
    struct ast_node *child;
    rf_ilist_for_each(&n->children, child, lh) {
        if (i >= 1) {
            it->lh = &n->children;
            it->ln = &child->lh;
            return child;
        }
        ++i;
    }
    return NULL;
}

struct ast_node *ast_matchexpr_next_case(const struct ast_node *n,
                                         struct ast_matchexpr_it *it)
{
    struct ast_node *ret;
    AST_NODE_ASSERT_TYPE(n, AST_MATCH_EXPRESSION);
    if (it->ln->next == &it->lh->n) {
        return NULL;
    }
    it->ln = it->ln->next;
    return rf_ilist_node_to_off(it->ln, rf_ilist_off_var(ret, lh));
}
