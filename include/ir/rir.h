#ifndef LFR_REFU_INTERMEDIATE_REPRESENTATION_H
#define LFR_REFU_INTERMEDIATE_REPRESENTATION_H

#include <stdbool.h>
#include <Data_Structures/intrusive_list.h>

struct analyzer;
struct RFstring;

struct rir {
    /* Memory pools */
    struct rf_fixed_memorypool *symbol_table_records_pool;
    struct rf_fixed_memorypool *types_pool;

    /* String tables containing identifiers and string literals in a file */
    struct string_table *identifiers_table;
    struct string_table *string_literals_table;

    //! A list of all composite types of the file
    struct RFilist_head composite_types;
    //! A list of all rir types of the file
    struct RFilist_head rir_types;

    //! The root of the ast node
    struct ast_node *root;
};

bool rir_init(struct rir *r, struct analyzer *a);
struct rir *rir_create(struct analyzer *a);

void rir_deinit(struct rir *r);
void rir_destroy(struct rir *r);

struct rir_module *rir_process(struct rir *r);


/* -- TODO Maybe move rir types list along with access functions into own struct -- */

/**
 * Searches the rir types list for a defined type called @c name
 *
 * @return The retrieved type or NULL if the type was not found
 */
struct rir_type * rir_types_list_get_defined(struct rir* r,
                                             const struct RFstring *name);
#endif
