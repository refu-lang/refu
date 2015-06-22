#ifndef LFR_MODULE_H
#define LFR_MODULE_H

#include <analyzer/string_table.h>

struct module {
    //! Pointer to the ast_node of the module
    struct ast_node *node;
    struct string_table import_names;
};

struct module *module_new(struct ast_node *n);
void module_destroy(struct module* m);

bool module_add_import(struct module *m, struct ast_node *import);
const struct RFstring *module_name(const struct module *m);

#endif
