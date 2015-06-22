#include <module.h>

#include <ast/ast.h>
#include <ast/module.h>

struct module *module_new(struct ast_node *n)
{
    struct module *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->node = n;
    if (!string_table_init(&ret->import_names)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void module_destroy(struct module* m)
{
    free(m);
}

bool module_add_import(struct module *m, struct ast_node *import)
{
    AST_NODE_ASSERT_TYPE(import, AST_IMPORT);
    struct ast_node *c;
    rf_ilist_for_each(&import->children, c, lh) {
        if (!string_table_add_or_get_str(&m->import_names, ast_identifier_str(c), NULL)) {
            return false;
        }
    }
    return true;
}


const struct RFstring *module_name(const struct module *m)
{
    return ast_module_name(m->node);
}
