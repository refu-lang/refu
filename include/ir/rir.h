#ifndef LFR_REFU_INTERMEDIATE_REPRESENTATION_H
#define LFR_REFU_INTERMEDIATE_REPRESENTATION_H

#include <stdbool.h>
#include <Data_Structures/intrusive_list.h>
#include "rir_types_list.h"

struct analyzer;
struct RFstring;
struct rf_objset_type;

struct rir {
    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;

    /* String tables containing identifiers and string literals in a file */
    struct string_table *identifiers_table;
    struct string_table *string_literals_table;

    //! A set of all types encountered (moved over from analyzer)
    struct rf_objset_type *types_set;
    //! A list of all rir types of the file
    struct rir_types_list rir_types_list;

    //! The root of the ast node
    struct ast_node *root;
};

bool rir_init(struct rir *r, struct analyzer *a);
struct rir *rir_create(struct analyzer *a);

bool rir_finalize_ast(struct rir *r);

void rir_deinit(struct rir *r);
void rir_destroy(struct rir *r);
#endif
