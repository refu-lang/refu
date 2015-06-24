#ifndef LFR_MODULE_H
#define LFR_MODULE_H

#include <Data_Structures/darray.h>
#include <RFintrusive_list.h>

struct module;
struct ast_node;
struct symbol_table;
struct analyzer;

//! Just a darray of ast modules
struct modules_arr {darray(struct module*);};
struct module {
    //! Pointer to the front_ctx that contains this module
    struct front_ctx *front;
    //! Owned pointer to the analyzer instance for this module
    struct analyzer *analyzer;
    //! Pointer to the ast_node of the module
    struct ast_node *node;
    //! A dynamic array of all the modules this module depends on
    struct modules_arr dependencies;
    //! Control, to add this module into the final sorted list of modules of the compiler
    struct RFilist_node ln;
};

/**
 * Create a new module and add it to the compiler's modules
 *
 * @param n            The root ast node of the module
 * @param front        The front_ctx containing the module
 * @return             The module in sucess or NULL in failure
 */
struct module *module_create(struct ast_node *n, struct front_ctx *front);
void module_destroy(struct module* m);

bool module_add_import(struct module *m, struct ast_node *import);
const struct RFstring *module_name(const struct module *m);
struct symbol_table *module_symbol_table(const struct module *m);
bool module_symbol_table_init(struct module *m);
struct inpfile *module_get_file(const struct module *m);
bool module_is_main(const struct module *m);
/**
 * Manually add the standard library as a dependency to a module
 */
bool module_add_stdlib(struct module *m);

bool module_analyze(struct module *m);

#endif
