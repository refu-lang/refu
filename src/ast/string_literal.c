#include <ast/string_literal.h>

#include <ast/ast.h>

struct ast_node *ast_string_literal_create(struct inplocation *loc)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_STRING_LITERAL, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->string_literal.string, loc->start.p,
                           loc->end.p - loc->start.p + 1);

    return ret;
}

i_INLINE_INS const struct RFstring *ast_string_literal_get_str(struct ast_node *lit);
