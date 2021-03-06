#ifndef LFR_TYPES_TYPE_COMPARISONS_H
#define LFR_TYPES_TYPE_COMPARISONS_H

#include <stdlib.h>
#include <rfbase/defs/inline.h>
#include <types/type_decls.h>

struct symbol_table;
struct module;

//! Flags to generally describe next type comparison
enum typecmp_flags {
    //! If the next comparison is for a function call
    TYPECMP_FLAG_FUNCTION_CALL = 0x1,
    TYPECMP_FLAG_ASSIGNMENT = 0x2,
};

enum comparison_reason {
    //! General type equality comparison
    TYPECMP_GENERIC = 0,
    //! Compare types not just for equality but for identical. Checks leaf names too
    TYPECMP_IDENTICAL,
    //! Compare types to check if implicit conversion is allowed
    TYPECMP_IMPLICIT_CONVERSION,
    //! Compare types inside a type after one implicit conversion has already happend
    TYPECMP_AFTER_IMPLICIT_CONVERSION,
    //! Compare types to check if explicit conversion is allowed
    TYPECMP_EXPLICIT_CONVERSION,
    //! Compare types for pattern matching. From is always the pattern type and to the match type
    TYPECMP_PATTERN_MATCHING
};


/**
 * Check if a type belong to a certain category. If it's a leaf type it's actual
 * type category is compared.
 */
i_INLINE_DECL bool type_category_equals(const struct type* t,
                                        enum type_category category)
{
    return t->category == category;
}

/**
 * Compare two types for a given reason.
 *
 * @warning For many comparison reasons type position does matter. So you may
 *          get different results if you do type_equals(A,B) and type_equals(B,A)
 *          Such reasons include assignment where for example u64 = u16 is ok but
 *          u16 = u64 is not allowed.
 *
 * @param from      The "from" Type is always the left part of the comparison.
 * @param to        The "to" Type is always the right part of the comparison.
 * @param reason    The reason for the comparison. @see enum comparison_reason
 * @return          True or false if comparisons succeeds or not.
 */
bool type_compare(
    const struct type *from,
    const struct type *to,
    enum comparison_reason reason
);

/**
 * Compare a type and an AST node that describes a type.
 * @param t         The type to compare
 * @param n         The node with which to compare @c t
 * @param a         The module for which the comparison happens
 * @param st        The symbol table to use in the comparison
 * @param genrdecl  An optional generic declaration that describes @c n.
 *                  Can be NULL.
 * @param options   Options to be passed to type_compare
 * @return          true if the type and the node describe the same type.
 *                  false otherwise.
 */
bool type_equals_ast_node(
    struct type *t,
    const struct ast_node *n,
    struct module *m,
    struct symbol_table *st,
    struct ast_node *genrdecl,
    enum comparison_reason options
);


/**
 * Will initialize all thread local data required to perform type comparisons
 */
bool typecmp_ctx_init();

/**
 * Will deinitialize all thread local data required to perform type comparisons
 */
void typecmp_ctx_deinit();

/**
 * @returns if we had an error string generated in the last failed comparison.
 *          Note that a comparison may fail without an error string generated.
 */
bool typecmp_ctx_have_error();

/**
 * @returns String explanation of the reason the last comparison error happened
 */
const struct RFstring *typecmp_ctx_get_error();

/**
 * If there are any warning this will return the next one.
 *
 * @returns A string describing the next warning or NULL if there are no more warnings
 */
const struct RFstring *typecmp_ctx_get_next_warning();

/**
 * @returns if we had a warning in the last comparison
 */
bool typecmp_ctx_have_warning();

/**
 * Sets bit flags for the next type comparison. Will reset comparison context.
 *
 * For possible values @see enum typecmp_flags
 */
void typecmp_ctx_set_flags(int flags);

/**
 * Will return the subtype the comparison matched to.
 *
 * This only makes sense if the "to" part of the comparison is a sum type
 * In all other cases the return will be NULL
 */
const struct type *typemp_ctx_get_matched_type();

#endif
