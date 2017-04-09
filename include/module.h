#ifndef LFR_MODULE_H
#define LFR_MODULE_H

#include <rfbase/datastructs/darray.h>
#include <rfbase/datastructs/intrusive_list.h>

#include <ast/ast_utils.h>
#include <utils/string_set.h>

struct module;
struct ast_node;
struct rir;
struct symbol_table;
struct analyzer;
struct type;

//! Just a darray of ast modules
struct modules_arr {darray(struct module*);};
struct module {
    //! Pointer to the front_ctx that contains this module
    struct front_ctx *front;
    //! The root ast_node of the module
    struct ast_node *node;
    //! The Refu IR format of the module
    struct rir *rir;
    //! A dynamic array of all the modules this module depends on
    struct modules_arr dependencies;
    //! A dynamic array of foreign functions this module needs
    struct arr_ast_nodes foreignfn_arr;
    //! Instantiated typeclasses for the module
    struct arr_ast_nodes instantiated_typeclasses;

    /* -- Members used only for the analysis stage of the module -- */
    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;
    //! A set of all types encountered
    struct rf_objset_type *types_set;
    /* String sets containing identifiers and string literals found during parsing */
    struct rf_objset_string identifiers_set;
    struct rf_objset_string string_literals_set;
    
    //! Control, to add this module into the final sorted list of modules of the compiler
    struct RFilist_node ln;
};

/**
 * Create a new module and add it to the compiler's modules
 *
 * @param n            The root ast node of the module, only given at creation
 *                     from AST parsing. Is NULL if coming from RIR parsing.
 * @param rir          The rir of the module, only given at creation
 *                     from RIR parsing. Is NULL if coming from AST parsing.
 * @param front        The front_ctx containing the module
 * @return             The module in sucess or NULL in failure
 */
struct module *module_create(struct ast_node *n, struct rir *rir, struct front_ctx *front);
void module_destroy(struct module* m);

/**
 * Add a foreign import statement's contents to a module
 *
 * @param m            The module to add the import to
 * @param import       The foreign import node to add
 */
void module_add_foreign_import(struct module *m, struct ast_node *import);
bool module_add_import(struct module *m, struct ast_node *import);

const struct RFstring *module_name(const struct module *m);
struct symbol_table *module_symbol_table(const struct module *m);

/**
 * Add a type instantiation to the module
 */
void module_add_type_instance(struct module *m, struct ast_node *typeinstance);

/**
 * Initializes the module symbol table iff there is a root node and that is
 * an ast module.
 */
bool module_symbol_table_init(struct module *m);
struct inpfile *module_get_file(const struct module *m);
bool module_is_main(const struct module *m);

/**
 * Returns the codepath that this module has taken to be created
 * as far as RIR is concerned. Either created it from AST or parsed it
 * directly from source.
 */
enum rir_pos module_rir_codepath(const struct module *m);

/**
 * Add a new type to the types set of this module
 *
 * @param m            The module to add the type to
 * @param new_type     The type to add
 * @param n            Not used for now, but it's a good idea to also provide
 *                     an optional ast node which to associate with the type.
 *                     Could remove in the future.
 */
bool module_types_set_add(struct module *m, struct type *new_type, const struct ast_node *n);

/**
 * Query if a module knows of a specific type by string representation
 *
 * @param m            The module to query
 * @param s            The string representation of the type to search for
 * @return             The type that was found or NULL if no type is found.
 */
struct type *module_types_set_has_str(const struct module *m, const struct RFstring *s);

/**
 * Manually add the standard library as a dependency to a module
 */
bool module_add_stdlib(struct module *m);

/**
 * Determine the dependencies of a module by checking the needed imports
 */
bool module_determine_dependencies(struct module *m, bool use_stdlib);

/**
 * Analyze the module
 *
 * This performs all of the parts of the analysis stage. Symbol table population,
 * typecheck, finalization
 */
bool module_analyze(struct module *m);

bool module_have_errors(const struct module *m);

#endif
