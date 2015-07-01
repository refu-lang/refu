#ifndef LFR_MODULE_H
#define LFR_MODULE_H

#include <Data_Structures/darray.h>
#include <RFintrusive_list.h>

struct module;
struct ast_node;
struct symbol_table;
struct analyzer;
struct type;

//! Just a darray of ast modules
struct modules_arr {darray(struct module*);};
struct module {
    //! Pointer to the front_ctx that contains this module
    struct front_ctx *front;
    //! Pointer to the ast_node of the module
    struct ast_node *node;
    //! A dynamic array of all the modules this module depends on
    struct modules_arr dependencies;

    /* -- Members used only for the analysis stage of the module -- */
    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;
    //! A set of all types encountered
    struct rf_objset_type *types_set;
    //! A list of all rir types of the file
    struct rir_types_list *rir_types_list;
    /* String tables containing identifiers and string literals found during parsing */
    struct string_table *identifiers_table;
    struct string_table *string_literals_table;
    
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
 * If existing, retrieve the type and if not existing create the type
 * for ast node @c desc
 *
 * @param m          The module containing the type
 * @param desc       The node whose type to check
 * @param st         The symbol table to check for type existence
 * @param genrdecl   Optional generic delcation that accompanied @c desc.
 *                   Can be NULL.
 * @return           The retrieved or created type, or NULL in error.
 */
struct type *module_get_or_create_type(struct module *m,
                                       const struct ast_node *desc,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl);

bool module_types_set_add(struct module *m, struct type *new_type);
/**
 * Manually add the standard library as a dependency to a module
 */
bool module_add_stdlib(struct module *m);

bool module_analyze(struct module *m);

#endif
