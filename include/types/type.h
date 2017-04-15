#ifndef LFR_TYPES_TYPE_H
#define LFR_TYPES_TYPE_H

#include <rfbase/utils/sanity.h>
#include <rfbase/defs/inline.h>
#include <rfbase/string/decl.h>

#include <utils/traversal.h>
#include <types/type_decls.h>
#include <types/type_elementary.h>

struct module;
struct symbol_table;
struct RFbuffer;
struct rf_fixed_memorypool;

void type_creation_ctx_init();
void type_creation_ctx_deinit();
/**
 * Set the common arguments type creation functions need for a single type
 *
 * @param m            The module for which to create the type
 * @param st           The symbol table to check for the type
 * @param genrdecl     An optional generic declaration node that describes @c n.
 *                     Can be NULL.
 */
void type_creation_ctx_set_args(
    struct module *m,
    struct symbol_table *st,
    struct ast_node *genrdecl
);

bool type_add_to_currop(struct type* t);

/* -- type allocation functions -- */

struct type *type_alloc(struct module *m);
struct type *type_alloc_copy(struct module *m, const struct type *source);
void type_free(struct type *t, struct rf_fixed_memorypool *pool);

/* -- various type creation and initialization functions -- */

// Arguments are set by @ref type_creation_ctx_set_args()
struct type *type_create_from_node(const struct ast_node *n);
// Arguments are set by @ref type_creation_ctx_set_args()
struct type *type_create_from_typedecl(const struct ast_node *n);
// Arguments are set by @ref type_creation_ctx_set_args()
struct type *type_create_from_fndecl(struct ast_node *n);
// Arguments are set by @ref type_creation_ctx_set_args()
struct type *type_create_from_typeelem(const struct ast_node *typedesc);
// Arguments are set by @ref type_creation_ctx_set_args()
struct type *type_operator_create_from_node(struct ast_node *n);

struct type *type_simple_create(
    enum type_category category,
    const struct RFstring *name
);

/* -- type getters -- */
/**
 * Attempts to retrieve the type for ast node @c n and if it does not exist
 * it creates it and adds it to the types set (if it's not already there)
 *
 * @note: If @c n is an ast description of a single type_leaf say a:f64 this is
 *        the kind of type this should return
 *
 * Arguments are set by @ref type_creation_ctx_set_args()
 *
 * @param n            The node whose type to retrieve/create
 * @return             Return either the type of @c n or NULL if there
 *                      was an error
 */
struct type *type_lookup_or_create(const struct ast_node *n);

/**
 * Applies a type operator to 2 types and returns the result. If either of the 2
 * parameter types is the same type_op then the type is appended instead of 
 * creating a new one. Also adds the type to the type set of the module if
 * a new type is created and does not exist in the module's types already.
 *
 * @param type          The type operator to apply to @c left and @c right
 * @param n             An ast node to add to the created symbol table entry
 * @param left          The type to become left part of the operand
 * @param right         The type to become right part of the operand
 * @param a             The module instance for which we are typechecking
 * @return              The new type or NULL if there was an error
 */
struct type *type_create_from_operation(
    enum typeop_type type,
    const struct ast_node *n,
    struct type *left,
    struct type *right,
    struct module *m
);

struct type *type_lookup_identifier_string(
    const struct RFstring *str,
    const struct symbol_table *st
);

/**
 * Lookup the type of an xidentifier
 *
 * Arguments are set by @ref type_creation_ctx_set_args()
 */
struct type *type_lookup_xidentifier(const struct ast_node *n);

/**
 * If existing, retrieve the type and if not existing create the type
 * for ast node @c desc
 *
 * Arguments are set by @ref type_creation_ctx_set_args()
 *
 * @param desc       The node whose type to check
 * @return           The retrieved or created type, or NULL in error.
 */
struct type *module_get_or_create_type(const struct ast_node *desc);

//! Options for invoking type_str()
enum type_str_options {
    TSTR_DEFAULT = 0x0,
    //! Print both the defined type's name and it's contents
    TSTR_DEFINED_WITH_CONTENTS,
    //! Print only the defined type's contents
    TSTR_DEFINED_ONLY_CONTENTS
};
/**
 * Gets a string representation of the type
 *
 * Before this function you need to use @ref RFS_PUSH() in order
 * to remember the temporary string buffer position and after it you need to
 * pop it with @ref RFS_POP().
 *
 * @param[in]  t            The type whose string representation to get
 * @param[in]  options      Bitflags that can have any of the options defined at
 *                          @ref type_str_options
 * @return                  Returns a pointer to a temporary string containing
 *                          the type's string representation or NULL in failure
 */
struct RFstring *type_str(const struct type *t, int options);
i_INLINE_DECL struct RFstring *type_str_or_die(const struct type *t, int options)
{
    struct RFstring *ret = type_str(t, options);
    if (!ret) {
        RF_CRITICAL("type_str() failure");
        exit(1);
    }
    return ret;
}

/**
 * Create a string representation of applying @a type to @a t1 and @a t2
 *
 * Before this function you need to use @ref RFS_PUSH() in order
 * to remember the temporary string buffer position and after it you need to
 * pop it with @ref RFS_POP().
 */
struct RFstring *type_op_create_str(
    const struct type *t1,
    const struct type *t2,
    enum typeop_type type
);

/**
 * Get a unique id for this type for use as a hash/key in data structures.
 *
 * TODO: Needs improvement
 *
 * @param t              The type whose unique key to get.
 */
size_t type_get_uid(const struct type *t);

/**
 * Query a unique type name for a type. If type is defined, its contents are used
 * in determining the unique string.
 *
 * @warning Needs to be enclosed in RFS_PUSH()/RFS_POP()
 */
const struct RFstring *type_get_unique_type_str(const struct type *t);

/**
 * @returns the wildcard type '_'
 */
const struct type *type_get_wildcard();

/**
 * @returns a generic type
 */
const struct type *type_get_generic();

i_INLINE_DECL bool type_is_defined(const struct type *t)
{
    return t->category == TYPE_CATEGORY_DEFINED;
}

/**
 * Gets the name of a defined type
 */
i_INLINE_DECL const struct RFstring *type_defined_get_name(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_DEFINED, "Called with non defined type category");
    return t->defined.name;
}

/**
 * Gets the containing type of a defined type
 */
i_INLINE_DECL struct type *type_defined_get_type(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_DEFINED, "Called with non defined type category");
    return t->defined.type;
}

#endif
