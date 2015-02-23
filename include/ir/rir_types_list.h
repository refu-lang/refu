#ifndef LFR_RIR_TYPES_LIST_H
#define LFR_RIR_TYPES_LIST_H

#include <stdbool.h>
#include <Data_Structures/intrusive_list.h>

struct RFstring;

struct rir_types_list {
    //! A list of all rir types of the file
    struct RFilist_head lh;
};

void rir_types_list_init(struct rir_types_list *t);
void rir_types_list_deinit(struct rir_types_list *t);

/**
 * Will populate the rir types list from the composite types
 *
 * RIR types are simply a non-tree form of types where each sum type is separated
 * into different types since in the backends we need to be able to easily distinguish
 * between sum types and their combinations.
 *
 * @param rir_types          The list of rir types to populate
 * @param composite_types    The list of types from which to create it
 *
 * @return                   true in success and false in failure
 */
bool rir_types_list_populate(struct rir_types_list *rir_types, struct RFilist_head *composite_types);

/**
 * Searches the rir types list for a defined type called @c name
 *
 * @return The retrieved type or NULL if the type was not found
 */
struct rir_type *rir_types_list_get_defined(struct rir_types_list *list,
                                            const struct RFstring *name);

/**
 * Convenience macro allowing simple iteration of rir_types
 */
#define rir_types_list_for_each(list_, type_) \
    rf_ilist_for_each(&(list_)->lh, type_, ln)
#endif
