#ifndef LFR_TYPES_TYPE_H
#define LFR_TYPES_TYPE_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>
#include <String/rf_str_decl.h>

#include <utils/traversal.h>
#include <types/type_decls.h>
#include <types/type_elementary.h>

struct module;
struct symbol_table;
struct RFbuffer;
struct rf_fixed_memorypool;

extern const struct RFstring g_wildcard_s;

void type_creation_ctx_init();
void type_creation_ctx_deinit();

bool type_add_to_currop(struct type* t);

/* -- type allocation functions -- */

struct type *type_alloc(struct module *m);
void type_free(struct type *t, struct rf_fixed_memorypool *pool);

/* -- various type creation and initialization functions -- */

struct type *type_create_from_node(const struct ast_node *n,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl);

struct type *type_create_from_typedecl(const struct ast_node *n,
                                       struct module *m,
                                       struct symbol_table *st);

struct type *type_create_from_fndecl(const struct ast_node *n,
                                     struct module *m,
                                     struct symbol_table *st);

struct type *type_function_create(struct module *m,
                                  struct type *arg_type,
                                  struct type *ret_type);

struct type *type_module_create(struct module *m, const struct RFstring *name);

struct type *type_create_from_typeelem(const struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl);

struct type *type_operator_create_from_node(struct ast_node *n,
                                            struct module *m,
                                            struct symbol_table *st,
                                            struct ast_node *genrdecl);

/* -- type getters -- */
/**
 * Attempts to retrieve the type for ast node @c n and if it does not exist
 * it creates it and adds it to the types set (if it's not already there)
 *
 * @note: If @c n is an ast description of a single type_leaf say a:f64 this is
 *        the kind of type this should return
 *
 * @param n            The node whose type to retrieve/create
 * @param m            The module for which to do it
 * @param st           The symbol table to check for the type
 * @param genrdecl     An optional generic declaration node that describes @c n.
 *                     Can be NULL.
 * @return             Return either the type of @c n or NULL if there was an error
 */
struct type *type_lookup_or_create(const struct ast_node *n,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl);

/**
 * Applies a type operator to 2 types and returns the result. If either of the 2
 * parameter types is the same type_op then the type is appended instead of 
 * creating a new one. Also adds the type to the type set of the module if
 * a new type is created an does not exist in the module's types already.
 *
 * @param type          The type operator to apply to @c left and @c right
 * @param left          The type to become left part of the operand
 * @param right         The type to become right part of the operand
 * @param a             The module instance for which we are typechecking
 * @return              The new type or NULL if there was an error
 */
struct type *type_create_from_operation(enum typeop_type type,
                                        struct type *left,
                                        struct type *right,
                                        struct module *m);

struct type *type_lookup_identifier_string(const struct RFstring *str,
                                           const struct symbol_table *st);
struct type *type_lookup_xidentifier(const struct ast_node *n,
                                     struct module *mod,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl);

//! Options for invoking type_str()
enum type_str_options {
    TSTR_DEFAULT = 0x0,
    TSTR_DEFINED_CONTENTS = 0x1  /*!< Print the contents of a defined user type if first */
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
struct RFstring *type_op_create_str(const struct type *t1,
                                    const struct type *t2,
                                    enum typeop_type type);

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

i_INLINE_DECL bool type_is_operator(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR;
}

i_INLINE_DECL struct type *typeop_to_type(struct type_operator *op)
{
    return container_of(op, struct type, operator);
}

/**
 * Query a type's operator type. If type is not an operator then TYPEOP_INVALID is returned
 */
i_INLINE_DECL enum typeop_type type_typeop_get(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR ? t->operator.type : TYPEOP_INVALID;
}

/**
 * Query if a type is a sum operator type
 */
i_INLINE_DECL bool type_is_sumop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_SUM;
}

/**
 * Query if a type is a product operator type
 */
i_INLINE_DECL bool type_is_prodop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_PRODUCT;
}

i_INLINE_DECL bool type_is_implop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_IMPLICATION;
}

/**
 * Query if a type is either a sum operator type or a defined type which contains
 * a sum of other types
 */
i_INLINE_DECL bool type_is_sumtype(const struct type *t)
{
    return type_is_sumop(t) ||
        (t->category == TYPE_CATEGORY_DEFINED && type_is_sumop(t->defined.type));
}

/**
 * @return the nth subtype of a type operation, or NULL if it does not exist or
 * if the type is not a type operator
 */
const struct type *type_get_subtype(const struct type *t, unsigned int index);

/**
 * @return the number of operands/subtypes a type operation has
 */
unsigned int type_get_subtypes_num(const struct type *t);

/**
 * Add a type as a subtype operand of a type operator
 */
void type_operator_add_operand(struct type_operator *p, struct type *c);
i_INLINE_DECL void type_add_operand(struct type *p, struct type *c)
{
    RF_ASSERT(p->category == TYPE_CATEGORY_OPERATOR, "Expected a type operator");
    return type_operator_add_operand(&p->operator, c);
}

/**
 * @return The index of @a t inside @a maybe_parent if it's found and -1 if not
 */
int type_is_direct_childof(const struct type *t, const struct type *maybe_parent);

/**
 * Works just like @ref type_is_direct_childof() but also allows for @a to be a
 * defined type
 */
i_INLINE_DECL int type_is_childof(const struct type *t, const struct type *maybe_parent)
{
    return maybe_parent->category == TYPE_CATEGORY_DEFINED
        ? type_is_direct_childof(t, maybe_parent->defined.type)
        : type_is_direct_childof(t, maybe_parent);
}
#endif
