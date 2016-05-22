#ifndef LFR_TYPES_SET_H
#define LFR_TYPES_SET_H

#include <rfbase/datastructs/objset.h>
#include <rfbase/defs/inline.h>

#include <types/type_decls.h>

struct type;
struct rir_type;
struct rf_fixed_memorypool;

i_INLINE_DECL const void *type_objset_key(const struct type *t)
{
    return (const void*)t;
}

size_t type_objset_hashfn(const struct type *t);
bool type_objset_eqfn(const struct type *t1,
                      const struct type *t2);

OBJSET_DEFINE_TYPE(type,
                   struct type,
                   type_objset_key,
                   type_objset_hashfn,
                   type_objset_eqfn)

/**
 * Check if a type can be converted to any type in the set
 *
 * @param set        The type set in question
 * @param type       The type to check if can be converted to any other type
 *                   in the set
 * @return           The type in the set that @a type can be converted to or
 *                   NULL for failure.
 */
struct type *type_objset_has_convertable(const struct rf_objset_type *set,
                                         const struct type *type);
/**
 * Check if a type UID has a corresponding type in the set
 *
 * @param set         The set in which to search.
 * @param uid         The UID to check for an existing type in the set
 * @return            Either a type that matches this uid or NULL if the set
 *                    does not contain such a type
 */
struct type *type_objset_has_uid(const struct rf_objset_type *set, size_t uid);

/**
 * Check if a type description as a string has a corresponding type in the set
 *
 * @param set         The set in which to search.
 * @param desc        The string description of a type to check if it already
 *                    exists in the set. The string description has to be
 *                    in the canonical way that type_str() would output it.
 */
struct type *type_objset_has_string(const struct rf_objset_type *set, const struct RFstring *desc);

void type_objset_destroy(struct rf_objset_type *set,
                         struct rf_fixed_memorypool *types_pool);

bool typeset_to_ordered_array(struct rf_objset_type *set, struct arr_types *arr);

#ifdef RF_OPTION_DEBUG
void type_objset_print(struct rf_objset_type *set);
#endif

#endif
