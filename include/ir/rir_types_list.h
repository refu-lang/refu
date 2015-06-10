#ifndef LFR_RIR_TYPES_LIST_H
#define LFR_RIR_TYPES_LIST_H

#include <stdbool.h>
#include <Data_Structures/intrusive_list.h>
#include <Data_Structures/objset.h>

struct RFstring;
struct type;
struct rf_objset_type;

/**
 * A global list of all rir types in a module
 */
struct rir_types_list {
    //! A list of all rir types of the file
    struct RFilist_head lh;
};

/**
 * Will initialize and populate the rir types list from the composite types
 *
 * RIR types are simply a non-tree form of types where each sum type is separated
 * into different types since in the backends we need to be able to easily distinguish
 * between sum types and their combinations.
 *
 * @param rir_types          The list of rir types to populate
 * @param types_set          The types set from which to create it
 *
 * @return                   true in success and false in failure
 */
bool rir_types_list_init(struct rir_types_list *t,
                         struct rf_objset_type *types_set);
void rir_types_list_deinit(struct rir_types_list *t);
struct rir_types_list *rir_types_list_create(struct rf_objset_type *types_set);
void rir_types_list_destroy(struct rir_types_list *t);

/**
 * Searches the rir types list for a defined type called @c name
 *
 * @return The retrieved type or NULL if the type was not found
 */
struct rir_type *rir_types_list_get_defined(struct rir_types_list *list,
                                            const struct RFstring *name);

/**
 * Searches the rir_types list for a type equal to a given type
 *
 * @param list        The rir types list
 * @param type        A normal type whose equivalent to search for in the list
 * @param name        An optional name to pass to @ref rir_type_equals_type() 
 *                    for the search
 * @return            The rir type or NULL if it was not found in the list.
 */
struct rir_type *rir_types_list_get_type(struct rir_types_list *list,
                                         const struct type *type,
                                         const struct RFstring *name);

/**
 * Convenience macro allowing simple iteration of rir_types
 */
#define rir_types_list_for_each(list_, type_) \
    rf_ilist_for_each(&(list_)->lh, type_, ln)

#endif
