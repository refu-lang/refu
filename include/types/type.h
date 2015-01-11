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

struct type *type_anonymous_create(struct ast_node *n,
                                   struct analyzer *a,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl);

struct type *type_create_from_typedesc(struct ast_node *typedesc,
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


bool type_equals_typedesc(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl);

struct type *type_lookup_identifier(struct ast_node *n,
                                    struct symbol_table *st);
struct type *type_lookup_xidentifier(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl);

/**
 * Gets a string representation of the type
 *
 * @param t             The type whose string representation to get
 * @param buff          If t is not a builtin type you can pass a buffer
 *                      here from which to initialize the string representation.
 *                      User should return the buffer to its previous state
 *                      right after use. Can also be NULL.
 * @return              Returns a pointer to the the string representation.
 *                      If there is an error returns NULL.
 */
const struct RFstring *type_str(const struct type *t, struct RFbuffer *buff);

/* -- type traversal functions -- */

typedef bool (*leaf_type_cb) (struct type_leaf *t, void *user_arg);

bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg);
#endif
