#include <ast/string_literal.h>

#include <ast/ast.h>

#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>

struct ast_node *ast_string_literal_create(struct inplocation *loc)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_STRING_LITERAL, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->string_literal.string, loc->start.p + 1,
                           loc->end.p - loc->start.p - 1);

    return ret;
}

bool ast_string_literal_hash_create(struct ast_node *n, struct analyzer *a)
{
        return string_table_add_str(a->string_literals_table,
                                    &n->string_literal.string,
                                    &n->string_literal.hash);
}

const struct RFstring *ast_string_literal_analyzed_str(const struct ast_node *n,
                                                       const struct analyzer *a)
{
    AST_NODE_ASSERT_TYPE(n, AST_STRING_LITERAL);
    return string_table_get_str(a->string_literals_table, n->string_literal.hash);
}

i_INLINE_INS const struct RFstring *ast_string_literal_get_str(struct ast_node *lit);
