#ifndef LFR_REFU_INTERMEDIATE_REPRESENTATION_H
#define LFR_REFU_INTERMEDIATE_REPRESENTATION_H

#include <stdbool.h>
#include <Data_Structures/intrusive_list.h>

struct analyzer;

struct rir {
    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;

    /* String tables containing identifiers and string literals in a file */
    struct string_table *identifiers_table;
    struct string_table *string_literals_table;

    //! A list of all composite types of the file
    struct RFilist_head composite_types;
};

bool rir_init(struct rir *r, struct analyzer *a);
struct rir *rir_create(struct analyzer *a);

void rir_deinit(struct rir *r);
void rir_destroy(struct rir *r);
#endif
