#ifndef LFR_TYPES_TYPE_H
#define LFR_TYPES_TYPE_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>
#include <String/rf_str_decl.h>

#include <types/type_decls.h>
struct analyzer;
struct symbol_table;
struct RFbuffer;


/* -- type allocation functions -- */

struct type *type_alloc(struct analyzer *a);
void type_free(struct type *t, struct analyzer *a);

/* -- various type creation and initialization functions -- */

struct type *type_create(struct ast_node *n, struct analyzer *a,
                         struct symbol_table *st,
                         struct ast_node *genrdecl);

struct type *type_create_from_typedecl(struct ast_node *n,
                                       struct analyzer *a,
                                       struct symbol_table *st);

struct type *type_create_from_vardecl(struct ast_node *n,
                                      struct analyzer *a,
                                      struct symbol_table *st);

struct type *type_create_from_fndecl(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st);

struct type *type_create_from_typedesc(struct ast_node *typedesc,
                                       struct analyzer *a,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl);

struct type *type_operator_create(struct ast_node *n,
                                  struct analyzer *a,
                                  struct symbol_table *st,
                                  struct ast_node *genrdecl);

/**
 * Attempts to retrieve the type for @c n and if it does not exist it creates it
 * @param n            The node whose type to retrieve/create
 * @param a            The analyzer instance for which to do it
 * @param st           The symbol table to check for the type
 * @param genrdecl     An optional generic declaration node that describes @c n.
 *                     Can be NULL.
 * @return             Return either the type of @c n or NULL if there was an error
 */
struct type *type_lookup_or_create(struct ast_node *n,
                                   struct analyzer *a,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl);

/* -- type comparison functions -- */

i_INLINE_DECL void type_comparison_ctx_init(struct type_comparison_ctx *ctx,
                                            enum comparison_reason reason)
{
    ctx->reason = reason;
    ctx->conversion = NO_CONVERSION;
    ctx->converted_type = NULL;
}

/**
 * Compare two types and see if they are equal or if one can be promoted to
 * the other
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

struct type *type_lookup_identifier_string(const struct RFstring *str,
                                           struct symbol_table *st);
struct type *type_lookup_xidentifier(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl);

/**
 * Gets a string representation of the type
 *
 * Before this function you need to execute use @ref RFS_buffer_push() in order
 * to remember the temporary string buffer position and after it you need to
 * pop it with @ref RFS_buffer_pop().
 *
 * @param t             The type whose string representation to get
 * @return              Returns a pointer to the the string representation.
 *                      If there is an error returns NULL.
 */
const struct RFstring *type_str(const struct type *t);

/* -- type traversal functions -- */

typedef bool (*leaf_type_cb) (struct type_leaf *t, void *user_arg);

bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg);
#endif
