#ifndef LFR_TYPECHECK_ARR_H
#define LFR_TYPECHECK_ARR_H

#include <utils/traversal.h>

struct type;
struct ast_node;
struct analyzer_traversal_ctx;

enum traversal_cb_res typecheck_bracketlist(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx
);

/**
 * Adjust the types of an array holding elementary values during assignment.
 *
 * If the bracket list contains elementary constants of a type different to
 * but convertable (if we reach this call they should be convertable) to the
 * the left type's array then this function iterates the constants of the
 * bracketlist and changes their type. It also changes the final type of
 * @a n.
 *
 * @param n          Should be a bracketlist ast node holding elementary types.
 * @param tleft      The left type of the assignment. Should also be also an
 *                   array type.
 */
void typecheck_adjust_elementary_arr_const_values(
    struct ast_node *n,
    const struct type *tleft
);


#endif
