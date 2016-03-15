#include <ast/string_literal.h>

#include <rflib/string/rf_str_core.h>

#include <ast/ast.h>
#include <module.h>

struct ast_node *ast_string_literal_create(struct inplocation *loc)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_STRING_LITERAL, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(
        &ret->string_literal.string,
        loc->start.p + 1,
        loc->end.p - loc->start.p - 1
    );
    ret->string_literal.hash = rf_hash_str_stable(&ret->string_literal.string, 0);

    return ret;
}

bool ast_string_literal_hash_create(struct ast_node *n, struct module *m)
{
    return rf_objset_add(&m->string_literals_set, string, &n->string_literal.string);
}

i_INLINE_INS const struct RFstring *ast_string_literal_get_str(const struct ast_node *lit);
i_INLINE_INS uint32_t ast_string_literal_get_hash(const struct ast_node *lit);
