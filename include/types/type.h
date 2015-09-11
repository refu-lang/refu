#ifndef LFR_TYPES_TYPE_H
#define LFR_TYPES_TYPE_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>
#include <String/rf_str_decl.h>

#include <utils/traversal.h>
#include <types/type_decls.h>
#include <types/type_elementary.h>
#include <ir/rir_type.h>

struct module;
struct symbol_table;
struct RFbuffer;

extern const struct RFstring g_wildcard_s;

/* -- type allocation functions -- */

struct type *type_alloc(struct module *m);
void type_free(struct type *t, struct module *m);

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

struct type *type_create_from_typedesc(struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl);
struct type *type_create_from_typeelem(const struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl);

struct type *type_operator_create(struct module *m,
                                  struct type *left_type,
                                  struct type *right_type,
                                  enum typeop_type type);

struct type *type_operator_create_from_node(struct ast_node *n,
                                            struct module *m,
                                            struct symbol_table *st,
                                            struct ast_node *genrdecl);

struct type *type_leaf_create(struct module *m,
                              const struct RFstring *id,
                              struct type *leaf_type);

struct type *type_leaf_create_from_node(const struct ast_node *typedesc,
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
 * @param make_leaf    If true and @c n is a type description of a single
 *                     type_leaf say a:f64 then this will return a type leaf.
 *                     If false then this will just return the right part of the
 *                     description
 * @return             Return either the type of @c n or NULL if there was an error
 */
struct type *type_lookup_or_create(const struct ast_node *n,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl,
                                   bool make_leaf);

/**
 * Applies a type operator to 2 types and returns the result
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

/**
 * Gets the rir vesion of a type.
 *
 * If the rir type is nil but type is elementary we retrieve it from the rir
 * elementary table. If it's not but rir type is stil nil then something is wrong
 * and we quit with an error.
 */
i_INLINE_DECL const struct rir_type *type_get_rir_or_die(const struct type *type)
{
    if (type->category == TYPE_CATEGORY_ELEMENTARY) {
        return rir_type_get_elementary(type_elementary(type));
    }
    if (!type->rir_type) {
        RF_CRITICAL_FAIL("Requested rir_type is NULL.");
    }
    return type->rir_type;
}

//! Options for invoking type_str()
enum type_str_options {
    TSTR_DEFAULT = 0x0,
    TSTR_LEAF_ID = 0x1,          /*!< Print the id of leaves */
    TSTR_DEFINED_CONTENTS = 0x2  /*!< Print the contents of a defined user type if first */
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
 * Get a unique id for this type for use as a hash/key in data structures.
 *
 * TODO: Needs improvement
 *
 * @param t              The type whose unique key to get.
 * @param count_leaf_id  If true then type's leaf ids contribute to uid generation
 */
size_t type_get_uid(const struct type *t, bool count_leaf_id);

/**
 * Query a unique value name for a type. If type is defined, its contents are used
 * in determining the unique string.
 *
 * @warning Needs to be enclosed in RFS_PUSH()/RFS_POP()
 */
const struct RFstring *type_get_unique_value_str(const struct type *t, bool count_leaf_id);

/**
 * Query a unique type name for a type. If type is defined, its contents are used
 * in determining the unique string.
 *
 * @warning Needs to be enclosed in RFS_PUSH()/RFS_POP()
 */
const struct RFstring *type_get_unique_type_str(const struct type *t, bool count_leaf_id);

/**
 * @returns the wildcard type '_'
 */
const struct type *type_get_wildcard();

/**
 * Gets the name of a defined type
 */
const struct RFstring *type_defined_get_name(const struct type *t);

/**
 * Query if a type is a sum operator type
 */
i_INLINE_DECL bool type_is_sumop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_SUM;
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
 * Get the nth type of a type operation, or kill the program if it doesn't exist
 *
 * @warning This function must be called on a type after typechecking. Only then
 * will the rir_types have been created allowing the function to work. If not 
 * the program will crush with an error.
 */
i_INLINE_DECL const struct type *type_get_nth_type_or_die(const struct type *t, unsigned int index)
{
    RF_ASSERT_OR_EXIT(t->rir_type, "Type's rir_type does not exist");
    const struct rir_type *rtype = rir_type_get_nth_type_or_die(t->rir_type, index);
    return rtype->type;
}

/**
 * Get the nth type of a type operation, or kill the program if it doesn't exist
 *
 * @warning This function must be called on a type after typechecking. Only then
 * will the rir_types have been created allowing the function to work. If not 
 * the program will crush with an error.
 */
i_INLINE_DECL const struct RFstring *type_get_nth_name_or_die(const struct type *t, unsigned int index)
{
    RF_ASSERT_OR_EXIT(t->rir_type, "Type's rir_type does not exist");
    return rir_type_get_nth_name_or_die(t->rir_type, index);
}

/* -- type traversal functions -- */

typedef bool (*type_iterate_cb) (struct type *t, void *user_arg);
typedef bool (*leaf_type_cb) (struct type_leaf *t, void *user_arg);
typedef enum traversal_cb_res (*leaf_type_nostop_cb) (const struct type_leaf *t, void *user_arg);

/**
 * Iterate and call callback for each subtype leaf
 * @param t          The type whose subtypes to iterate
 * @param cb         The callback to call on each type leaf
 * @param user_arg   The input to the callback
 *
 * @return true for success and false if the callback fails anywhere
 */
bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg);

/**
 * Behaves just like @c type_for_each_leaf() but has different treatment of
 * callback return.
 * @param t          The type whose subtypes to iterate
 * @param cb         The callback to call on each type leaf
 * @param user_arg   The input to the callback
 *
 * @return true for success and false if the callback fails anywhere
 */
enum traversal_cb_res type_for_each_leaf_nostop(const struct type *t, leaf_type_nostop_cb cb, void *user_arg);

/**
 * Post order iteration of a given type
 * @param t            The type to traverse
 * @param cb           The callback to execute in a post order fashion for each node
 *                     of the type description
 * @param user_arg     An extra argument to provide to the callbacks
 *
 * @return             true if all callbacks returned succesfully and false otherwise
 */
bool type_traverse_postorder(struct type *t, type_iterate_cb cb, void *user_arg);

/**
 * Tteration of a given type, with callbacks both for when going down and up.
 * Traversal should be in order.
 * @param t            The type to traverse
 * @param pre_cb       The callback to execute in a pre order fashion for each node
 *                     of the type description
 * @param post_cb      The callback to execute in a post order fashion for each node
 *                     of the type description
 * @param user_arg     An extra argument to provide to the callbacks
 *
 * @return             true if all callbacks returned succesfully and false otherwise
 */
bool type_traverse(struct type *t, type_iterate_cb pre_cb,
                   type_iterate_cb post_cb, void *user_arg);

#endif
