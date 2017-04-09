#ifndef LFR_ANALYZER_SYMBOL_TABLE_H
#define LFR_ANALYZER_SYMBOL_TABLE_H

#include <rfbase/utils/sanity.h>
#include <rfbase/datastructs/htable.h>
#include <rfbase/string/decl.h>
#include <rfbase/defs/inline.h>

#include <types/type_decls.h>

struct ast_node;
struct RFstring;
struct symbol_table;
struct type;
struct rir_type;
struct rir_types_list;
struct module;

/* -- symbol table record functionality -- */

struct symbol_table_record {
    //! The identifier string used as the key to the symbol table
    const struct RFstring *id;
    //! [optional] The ast node the identifier should point to
    //! Can actually be NULL.
    const struct ast_node *node;
    //! Description of the type the identifier refers to
    //! Can be NULL only if referring to a typeclass
    struct type *data;
    //! The rir object used for this symbol, or NULL if not set
    struct rir_object *rirobj;
};

bool symbol_table_record_init(
    struct symbol_table_record *rec,
    struct module *mod,
    struct symbol_table *st,
    const struct ast_node *node,
    const struct RFstring *id
);

struct symbol_table_record *symbol_table_record_create(
    struct symbol_table *st,
    struct module *mod,
    const struct ast_node *node,
    const struct RFstring *id);

struct symbol_table_record *symbol_table_record_create_from_type(
    struct symbol_table *st,
    const struct RFstring *id,
    struct type *t);

void symbol_table_record_print(const struct symbol_table_record *rec);

void symbol_table_record_destroy(struct symbol_table_record *rec,
                                 struct symbol_table *st);

i_INLINE_DECL const struct RFstring *
symbol_table_record_id(struct symbol_table_record *rec)
{
    return rec->id;
}

i_INLINE_DECL const struct ast_node *
symbol_table_record_node(struct symbol_table_record *rec)
{
    return rec->node;
}

i_INLINE_DECL struct type *
symbol_table_record_type(struct symbol_table_record *rec)
{
    return rec->data;
}

/* -- symbol table functionality -- */

struct symbol_table {
    //! Hash table of symbols
    struct htable table;
    //! Pointer to the parent symbol table, or NULL if this is the top table
    struct symbol_table *parent;
    //! Pointer to the module of this symbol table. Used only from the top symbol
    //! table of a module to check for symbols in dependendencies
    struct module *mod;
    //! Pointer to the analyzer instance's memory pool or if this is
    //! the root's symbol table an owned pointer to the symbol table
    struct rf_fixed_memorypool *pool;
    //! Pointer to the function declaration that contains this table, or NULL
    //! if this is the top table
    struct ast_node *fndecl;
};

bool symbol_table_init(struct symbol_table *t, struct module *m);
bool root_symbol_table_init(struct symbol_table *t);
void symbol_table_deinit(struct symbol_table *t);

/**
 * Add a node to the symbol table and also set its type
 */
bool symbol_table_add_node(
    struct symbol_table *t,
    struct module *m,
    const struct RFstring *id,
    struct ast_node *n
);

bool symbol_table_add_type(
    struct symbol_table *st,
    struct module *mod,
    const struct RFstring *id,
    struct type *t,
    const struct ast_node *node_desc
);

bool symbol_table_add_record(
    struct symbol_table *t,
    struct symbol_table_record *rec
);

bool symbol_table_add_genrdecl(
    struct symbol_table *t,
    struct module *mod,
    struct ast_node *gen
);

/**
 * Lookup an ast_node in a symbol table
 * Arguments are just like @rec symbol_table_lookup_record()
 * @return  The found ast_node or NULL for failure
 */
const struct ast_node *symbol_table_lookup_node(
    const struct symbol_table *t,
    const struct RFstring *id,
    bool *at_first_symbol_table
);
/**
 * Lookup a record in a symbol table
 *
 * @param t                        The symbol table in which to lookup the record
 * @param id                       The identifier (key) with which to perform the lookup
 * @param at_first_symbol_table    If not NULL then this is passing a boolean which
 *                                 which return whether or not the record was foun
 *                                 in @c t itself (true) or in any of its parents (false)
 * @return                         The symbol table record or NULL for failure.
 */
struct symbol_table_record *symbol_table_lookup_record(
    const struct symbol_table *t,
    const struct RFstring *id,
    bool *at_first_symbol_table);

/**
s * Lookup a typedesc node in a symbol table. This function is to be used only
 * in special cases like in a match case where you have a symbol table
 * and a case expression and you need to get the symbol table record for that
 * expression.
 *
 * @param t                        The symbol table in which to lookup the record
 * @param typedesc                 The type description node to search for
 * @param at_first_symbol_table    If not NULL then this is passing a boolean which
 *                                 which return whether or not the record was foun
 *                                 in @c t itself (true) or in any of its parents (false)
 * @return                         The symbol table record or NULL for failure.
 */
struct symbol_table_record *symbol_table_lookup_typedesc(
    const struct symbol_table *t,
    const struct ast_node *typedesc,
    bool *at_first_symbol_table);

/**
 * Lookup a defined type by name in the symbol table
 *
 * @param t                        The symbol table in which to lookup
 * @param id                       The name of the defined type to look for
 * @param at_first_symbol_table    If not NULL then this is passing a boolean which
 *                                 which return whether or not the record was foun
 *                                 in @c t itself (true) or in any of its parents (false)
 * @return                         The defined type or NULL for not found.
 */
struct type *symbol_table_lookup_defined_type(
    const struct symbol_table *t,
    const struct RFstring *id,
    bool *at_first_symbol_table);

/**
 * Lookup if a rir object has an entry in a symbol table or its parents
 *
 *
 * @param t                         The symbol table in which to look
 * @param obj                       The rir object to search for in the symbol
 *                                  table.
 */
struct symbol_table_record *symbol_table_lookup_rirobj(
    const struct symbol_table *t,
    struct rir_object *obj);

void symbol_table_iterate(struct symbol_table *t, htable_iter_cb cb, void *user);

i_INLINE_DECL void symbol_table_set_parent(
    struct symbol_table *t,
    struct symbol_table *parent)
{
    t->parent = parent;
}

i_INLINE_DECL void symbol_table_set_fndecl(
    struct symbol_table *t,
    struct ast_node *decl)
{
    t->fndecl = decl;
}

i_INLINE_DECL struct ast_node *symbol_table_get_fndecl(struct symbol_table *t)
{
    return t->fndecl;
}

i_INLINE_DECL bool symbol_table_is_root(const struct symbol_table *t)
{
    return t->mod == NULL;
}

i_INLINE_DECL bool symbol_table_is_empty(const struct symbol_table *t)
{
    return htable_is_empty(&t->table);
}

/**
 * Convenience function to help swap a parent symbol table with its child while
 * traversing the AST downwards in symbol table creation. Never try to swap 
 * a symbol table with itself.
 */
i_INLINE_DECL void symbol_table_swap_current(
    struct symbol_table **current_st_ptr,
    struct symbol_table *new_st)
{
    RF_ASSERT(*current_st_ptr != new_st, "Tried to swap a symbol table with itself");
    symbol_table_set_parent(new_st, *current_st_ptr);
    symbol_table_set_fndecl(new_st, (*current_st_ptr)->fndecl);
    *current_st_ptr = new_st;
}

i_INLINE_DECL struct type *symbol_table_lookup_type(
    struct symbol_table *t,
    const struct RFstring *id,
    bool *at_first_symbol_table)
{
    struct symbol_table_record *rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    if (!rec) {
        return NULL;
    }
    return rec->data;
}

/**
 * Check if the symbol table has a typeclass declaration/instance symbol table
 * as a parent and if it does return it.
 *
 * @param st       The symbol table from which to start the search
 * @return         The typeclass/typeclass_instance node if found and NULL otherwise
 */
struct ast_node *symbol_table_has_typeclass_parent(const struct symbol_table *st);

/**
 * Checks if the `self` special identifier makes sense in this symbol table and
 * if it does returns its type
 *
 * @param st        The symbol table from which to search
 * @return          The type of `self` in this context or NULL if self can't
 *                  be used in this context.
 */
const struct type *symbol_table_check_and_get_selftype(const struct symbol_table *st);

/**
 * Checks if the `self` special identifier makes sense in this symbol table and
 * if it does returns the ast node declaring the type it instantiates
 *
 * @param st        The symbol table from which to search
 * @return          The ast node declaration for type of `self` in this context
 *                  or NULL if self can't be used in this context.
 */
const struct ast_node *symbol_table_check_and_get_selftypedecl(const struct symbol_table *st);

#ifdef RF_OPTION_DEBUG
/**
 * Print all records inside a symbol table. Used only for debugging
 */
void symbol_table_print(struct symbol_table *t);
#endif

#endif
