#ifndef LFR_TYPES_TYPE_COMPARISONS_H
#define LFR_TYPES_TYPE_COMPARISONS_H

#include <stdlib.h>
#include <Definitions/inline.h>
#include <types/type_decls.h>

struct symbol_table;
struct analyzer;

enum conversion_type {
    NO_CONVERSION = 0x0,
    SIGNED_TO_UNSIGNED = 0x1,
    LARGER_TO_SMALLER = 0X2,
};

enum comparison_reason {
    COMPARISON_REASON_GENERIC,   /*!< General type equality comparison */
    COMPARISON_REASON_IDENTICAL, /*!< Compare types not just for equality but for identical. Checks leaf names too. */

    COMPARISON_REASON_ASSIGNMENT,
    COMPARISON_REASON_ADDITION,
    COMPARISON_REASON_SUBTRACTION,
    COMPARISON_REASON_MULTIPLICATION,
    COMPARISON_REASON_DIVISION,

    COMPARISON_REASON_FUNCTION_CALL,
};

struct type_comparison_ctx {
    //! The reason for the request of
    enum comparison_reason reason;
    //! Query to see what conversions happened. Can contain multiple bitflags
    enum conversion_type conversion;
    //! If any conversion happened this should point to the converted type
    struct type *converted_type;
    //! If we have a binary operation this shold point to the common type
    const struct type *common_type;
};

i_INLINE_DECL void type_comparison_ctx_init(struct type_comparison_ctx *ctx,
                                            enum comparison_reason reason)
{
    ctx->reason = reason;
    ctx->conversion = NO_CONVERSION;
    ctx->converted_type = NULL;
    ctx->common_type = NULL;
}

/**
 * Get the reason of the comparison from the context or generic reason if it's NULL
 */
i_INLINE_DECL enum comparison_reason type_comparison_ctx_reason(struct type_comparison_ctx *ctx)
{
    return ctx ? ctx->reason : COMPARISON_REASON_GENERIC;
}

/**
 * Check if a type belong to a certain category. If it's a leaf type it's actual
 * type category is compared.
 */
i_INLINE_DECL bool type_category_equals(const struct type* t,
                                        enum type_category category)
{
    return t->category == category ||
           (t->category== TYPE_CATEGORY_LEAF && t->leaf.type->category == category);
}

/**
 * Compare two types and see if they are equal or if one can be promoted to
 * the other.
 *
 * @todo TODO: Context being optionally NULL introduces a lot of problems and
 *             additional checks in every step of the way. Maybe make it mandatory?
 *
 * @warning For many comparison reasons type position does matter. So you may
 *          get different results if you do type_equals(A,B) and type_equals(B,A)
 *          Such reasons include assignment where for example u64 = u16 is ok but
 *          u16 = u64 is not allowed.
 *
 * @param t1        Type 1 for comparison
 * @param t2        Type 2 for comparison
 * @param ctx       Type comparison context passed by the user of the function.
 *                  Should be initialized with type_comparison_ctx_init().
 *                  Check @c type_comparison_ctx for description.
 *                  Can be NULL if all we want is a simple type check.
 * @return          True if they are perfectly equal or if they can be equal
 *                  through conversions. In the second case @c ctx is set
 *                  accordingly. False for mismatch.
 */
bool type_equals(const struct type* t1, const struct type *t2,
                 struct type_comparison_ctx *ctx);

/**
 * Compare a type and an AST node that describes a type.
 * @param t         The type to compare
 * @param n         The node with which to compare @c t
 * @param a         The analyzer instance
 * @param st        The symbol table to use in the comparison
 * @param genrdecl  An optional generic declaration that describes @c n.
 *                     Can be NULL.
 * @return          true if the type and the node describe the same type.
 *                  false otherwise.
 */
bool type_equals_ast_node(struct type *t, struct ast_node *n,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl);

#endif
