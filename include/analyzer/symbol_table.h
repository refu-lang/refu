#ifndef LFR_ANALYZER_SYMBOL_TABLE_H
#define LFR_ANALYZER_SYMBOL_TABLE_H

#include <Data_Structures/htable.h>
#include <String/rf_str_decl.h>

struct ast_node;
struct RFstring;

struct symbol_table {
    struct htable table;
};

struct symbol_table_record {
    struct ast_node *type;
    struct RFstring id;
};

bool symbol_table_init(struct symbol_table *t);
void symbol_table_deinit(struct symbol_table *t);

bool symbol_table_add(struct symbol_table *t,
                      const struct RFstring *id,
                      struct ast_node *n);

struct ast_node *symbol_table_lookup(struct symbol_table *t,
                                     const struct RFstring *id);

#endif
