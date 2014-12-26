#ifndef LFR_ANALYZER_SYMBOL_TABLE_H
#define LFR_ANALYZER_SYMBOL_TABLE_H

#include <Data_Structures/htable.h>
#include <String/rf_str_decl.h>
#include <Definitions/inline.h>
#include <types/type.h>

struct analyzer;
struct ast_node;
struct RFstring;
struct symbol_table;

/* -- symbol table record functionality -- */

struct symbol_table_record {
    //! The identifier string used as the key to the symbol table
    const struct RFstring *id;
    //! [optional] The ast node the identifier should point to
    //! Can actually be NULL.
    struct ast_node *node;
    //! Description of the type the identifier refers to
    struct type *data;
};

bool symbol_table_record_init(struct symbol_table_record *rec,
                              struct analyzer *analyzer,
                              struct symbol_table *st,
                              struct ast_node *node,
                              const struct RFstring *id);

struct symbol_table_record *symbol_table_record_create(
    struct symbol_table *st,
    struct analyzer *analyzer,
    struct ast_node *node,
    const struct RFstring *id);

void symbol_table_record_destroy(struct symbol_table_record *rec,
                                 struct symbol_table *st);

i_INLINE_DECL const struct RFstring *
symbol_table_record_id(struct symbol_table_record *rec)
{
    return rec->id;
}

i_INLINE_DECL enum type_category
symbol_table_record_category(struct symbol_table_record *rec)
{
    return rec->data->category;
}

i_INLINE_DECL struct ast_node *
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
    struct htable table;
    struct symbol_table *parent;
    struct rf_fixed_memorypool *pool;
};

bool symbol_table_init(struct symbol_table *t, struct analyzer *a);
void symbol_table_deinit(struct symbol_table *t);

bool symbol_table_add_node(struct symbol_table *t,
                           struct analyzer *analyzer,
                           const struct RFstring *id,
                           struct ast_node *n);

bool symbol_table_add_type(struct symbol_table *st,
                           struct analyzer *analyzer,
                           const struct RFstring *id,
                           struct type *t);

bool symbol_table_add_record(struct symbol_table *t,
                             struct symbol_table_record *rec);

struct ast_node *symbol_table_lookup_node(struct symbol_table *t,
                                          const struct RFstring *id,
                                          bool *at_first_symbol_table);

struct symbol_table_record *symbol_table_lookup_record(struct symbol_table *t,
                                                       const struct RFstring *id,
                                                       bool *at_first_symbol_table);

void symbol_table_iterate(struct symbol_table *t, htable_iter_cb cb, void *user);

i_INLINE_DECL void symbol_table_set_parent(struct symbol_table *t,
                                           struct symbol_table *parent)
{
    t->parent = parent;
}


i_INLINE_DECL struct type *symbol_table_lookup_type(struct symbol_table *t,
                                                    const struct RFstring *id,
                                                    bool *at_first_symbol_table)
{
    struct symbol_table_record *rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    if (!rec) {
        return NULL;
    }

    return rec->data;
}

#endif
