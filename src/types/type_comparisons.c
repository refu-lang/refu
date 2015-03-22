#include <types/type_comparisons.h>

#include <Utils/sanity.h>
#include <Utils/bits.h>
#include <Definitions/threadspecific.h>
#include <String/rf_str_common.h>
#include <String/rf_str_corex.h>
#include <Data_Structures/darray.h>

#include <ast/ast.h>
#include <ast/type.h>

#include <types/type.h>
#include <types/type_elementary.h>

/* -- typecmp_ctx functions -- */

struct typecmp_ctx {
    bool needs_reset;
    int flags;
    struct RFstringx err_buff;
    struct RFstringx warn_buff;
    struct {darray(struct RFstring);} warning_indices;
};

i_THREAD__ struct typecmp_ctx g_typecmp_ctx;

#define TYPECMP_RETURN(i_retvalue_)                   \
    g_typecmp_ctx.needs_reset = true;                 \
    return i_retvalue_


bool typecmp_ctx_init()
{
    darray_init(g_typecmp_ctx.warning_indices);
    g_typecmp_ctx.needs_reset = false;
    g_typecmp_ctx.flags = 0;
    return rf_stringx_init_buff(&g_typecmp_ctx.err_buff, 1024, "") &&
        rf_stringx_init_buff(&g_typecmp_ctx.warn_buff, 1024, "");
}

static inline void typecmp_ctx_reset()
{
    if (g_typecmp_ctx.needs_reset) {
        g_typecmp_ctx.needs_reset = false;
        g_typecmp_ctx.flags = 0;
        rf_stringx_assignv(&g_typecmp_ctx.err_buff, "");
        rf_stringx_assignv(&g_typecmp_ctx.warn_buff, "");
        while (g_typecmp_ctx.warning_indices.size != 0) {
            (void)darray_pop(g_typecmp_ctx.warning_indices);
        }
    }
}

void typecmp_ctx_deinit()
{
    darray_free(g_typecmp_ctx.warning_indices);
    rf_stringx_deinit(&g_typecmp_ctx.err_buff);
    rf_stringx_deinit(&g_typecmp_ctx.warn_buff);
}

const struct RFstring *typecmp_ctx_get_error()
{
    return &g_typecmp_ctx.err_buff.INH_String;
}

const struct RFstring *typecmp_ctx_get_next_warning()
{
    if (g_typecmp_ctx.warning_indices.size == 0) {
        return NULL;
    }

    return &darray_pop(g_typecmp_ctx.warning_indices);
}

bool typecmp_ctx_have_error()
{
    return !rf_string_is_empty(&g_typecmp_ctx.err_buff);
}

bool typecmp_ctx_have_warning()
{
    return !rf_string_is_empty(&g_typecmp_ctx.warn_buff);
}

static inline void typecmp_ctx_add_warning(struct RFstring *s)
{
    darray_resize(g_typecmp_ctx.warning_indices, g_typecmp_ctx.warning_indices.size + 1);
    RF_STRING_SHALLOW_INIT(
        &darray_top(g_typecmp_ctx.warning_indices),
        (rf_string_is_empty(&g_typecmp_ctx.warn_buff)) ? g_typecmp_ctx.warn_buff.INH_String.data :
            g_typecmp_ctx.warn_buff.INH_String.data + g_typecmp_ctx.warn_buff.INH_String.length,
        rf_string_length_bytes(s));
    rf_stringx_append(&g_typecmp_ctx.warn_buff, s);
}

void typecmp_ctx_set_flags(int flags)
{
	typecmp_ctx_reset();
    g_typecmp_ctx.flags = flags;
}

/* -- type comparison functions -- */

i_INLINE_INS bool type_category_equals(const struct type* t,
                                       enum type_category category);
static inline bool type_leaf_compare(const struct type_leaf *from,
                                     const struct type_leaf *to,
                                     enum comparison_reason reason)
{
    bool predicate = true;
    if (reason == TYPECMP_IDENTICAL) {
        predicate = rf_string_equal(from->id, to->id);
    }
    return predicate && type_compare(from->type, to->type, reason);
}

static bool type_operator_compare(const struct type_operator *from,
                                  const struct type_operator *to,
                                  enum comparison_reason reason)
{
    if (from == to) {
        TYPECMP_RETURN(true);
    }

    if (from->type != to->type) {
        TYPECMP_RETURN(false);
    }

    return type_compare(from->left, to->left, reason) &&
           type_compare(from->right, to->right, reason);
}

static bool type_elementary_compare(const struct type_elementary *from,
                                    const struct type_elementary *to,
                                    enum comparison_reason reason)
{
    bool fail_due_to_second_implicit = false;
    if (from->etype == to->etype) {
        TYPECMP_RETURN(true);
    }

    if (reason == TYPECMP_IDENTICAL) {
        goto end;
    }

    // we must then be looking at either implicit or explicit conversion
    switch (from->etype) {
    case ELEMENTARY_TYPE_INT:
    case ELEMENTARY_TYPE_UINT:
    case ELEMENTARY_TYPE_INT_8:
    case ELEMENTARY_TYPE_UINT_8:
    case ELEMENTARY_TYPE_INT_16:
    case ELEMENTARY_TYPE_UINT_16:
    case ELEMENTARY_TYPE_INT_32:
    case ELEMENTARY_TYPE_UINT_32:
    case ELEMENTARY_TYPE_INT_64:
    case ELEMENTARY_TYPE_UINT_64:
        // int to int
        if (type_elementary_is_int(to)) {
            if (!from->is_constant) {
                // warn about conversion from bigger to smaller type
                if (reason != TYPECMP_EXPLICIT_CONVERSION &&
                    type_elementary_bytesize(from) > type_elementary_bytesize(to)) {

                    RFS_buffer_push();
                    typecmp_ctx_add_warning(
                        RFS_(
                            "Implicit conversion from \""RF_STR_PF_FMT"\" to \""
                            RF_STR_PF_FMT"\"",
                            RF_STR_PF_ARG(type_elementary_get_str(from->etype)),
                            RF_STR_PF_ARG(type_elementary_get_str(to->etype))));
                    RFS_buffer_pop();
                }
                // warn about conversion from signed to unsigned
                if (reason != TYPECMP_EXPLICIT_CONVERSION &&
                    !type_elementary_int_is_unsigned(from) &&
                    type_elementary_int_is_unsigned(to)) {

                    RFS_buffer_push();
                    typecmp_ctx_add_warning(
                        RFS_(
                            "Implicit signed to unsigned conversion from \""RF_STR_PF_FMT"\" "
                            "to \""RF_STR_PF_FMT"\"",
                            RF_STR_PF_ARG(type_elementary_get_str(from->etype)),
                            RF_STR_PF_ARG(type_elementary_get_str(to->etype))));
                    RFS_buffer_pop();
                }
            }
            TYPECMP_RETURN(true);
        }

        if (type_elementary_is_float(to)) {// an int is convertible to a float
            TYPECMP_RETURN(true);
        }

        if (to->etype == ELEMENTARY_TYPE_BOOL) { // we can convert from int to bool
            TYPECMP_RETURN(true);
        }

        // explicit conversion from int constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            from->is_constant &&
            reason == TYPECMP_EXPLICIT_CONVERSION) {
            
            TYPECMP_RETURN(true);
        }
        break;
    case ELEMENTARY_TYPE_FLOAT_32:
    case ELEMENTARY_TYPE_FLOAT_64:
        // float to float is okay
        if (type_elementary_is_float(to)) {
            TYPECMP_RETURN(true);
        }

        // only explicit float to int
        if (type_elementary_is_int(to) && reason == TYPECMP_EXPLICIT_CONVERSION) {
            TYPECMP_RETURN(true);
        }

        // explicit conversion from float constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            from->is_constant &&
            reason == TYPECMP_EXPLICIT_CONVERSION) {
            
            TYPECMP_RETURN(true);
        }
        break;
    case ELEMENTARY_TYPE_BOOL:
        if (reason == TYPECMP_AFTER_IMPLICIT_CONVERSION) {
            // if we only had an implicit conversion bool is not convertable
            fail_due_to_second_implicit = true;
            break;
        }
            if (type_elementary_is_int(to)) { // we can convert from bool to int
                TYPECMP_RETURN(true);
            }
            // we can explicitly convert from bool to string
            if (to->etype == ELEMENTARY_TYPE_STRING && reason == TYPECMP_EXPLICIT_CONVERSION) {
                TYPECMP_RETURN(true);
            }
        break;
    case ELEMENTARY_TYPE_STRING:
    case ELEMENTARY_TYPE_NIL:
        break;
    default:
        RF_ASSERT(false, "Invalid elementary type at comparisons");
        break;
    }

end:
    rf_stringx_assignv(&g_typecmp_ctx.err_buff,
                       "Unable to convert from \""RF_STR_PF_FMT"\" to \""
                       RF_STR_PF_FMT"\"%s",
                       RF_STR_PF_ARG(type_elementary_get_str(from->etype)),
                       RF_STR_PF_ARG(type_elementary_get_str(to->etype)),
                       fail_due_to_second_implicit ? ". An implicit conversion already happened" : ""
    );
    TYPECMP_RETURN(false);
}

static bool type_same_categories_compare(const struct type *from,
                                         const struct type *to,
                                         enum comparison_reason reason)
{
    RF_ASSERT(from->category == to->category, "type_same_categories_equals() called "
              "with types of different categories");
    switch (from->category) {
    case TYPE_CATEGORY_OPERATOR:
        return type_operator_compare(&from->operator, &to->operator, reason);
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_compare(&from->elementary, &to->elementary, reason);
    case TYPE_CATEGORY_LEAF:
        return type_leaf_compare(&from->leaf, &to->leaf, reason);
    case TYPE_CATEGORY_DEFINED:
        return rf_string_equal(from->defined.name, to->defined.name) &&
            type_compare(from->defined.type, to->defined.type, reason);
    case TYPE_CATEGORY_GENERIC:
        //TODO
        RF_ASSERT(false, "Not yet implemented");
        break;
    default:
        RF_ASSERT(false, "Illegal type category");
        break;
    }

    TYPECMP_RETURN(false);
}

static bool type_compare_to_operator(const struct type *from,
									 const struct type_operator *to,
                                     enum comparison_reason reason)
{
    if (to->type == TYPEOP_SUM) {
        return type_compare(from, to->left, TYPECMP_AFTER_IMPLICIT_CONVERSION) || type_compare(from, to->right, TYPECMP_AFTER_IMPLICIT_CONVERSION);
    }
    return false;
}

//! Possible resuls of @see type_initial_check()
enum type_initial_check_result {
    TYPES_ARE_NOT_EQUAL,
    TYPES_ARE_EQUAL,
    TYPES_CHECK_CAN_CONTINUE,
};
/**
 *  Performs the first step of type comparison making sure that after its call
 *  one of 3 things happen:
 *
 *  * @c TYPES_ARE_NOT_EQUAL:          The comparison fails immediately
 *  * @c TYPES_ARE_EQUAL:              The comparison succeeds
 *  * @c TYPES_CHECK_CAN_CONTINUE:     Made sure compared types are of same category
 *                                     and type equality comparison can proceed.
 */
static inline enum type_initial_check_result type_category_check(const struct type *from,
                                                                 const struct type *to,
                                                                 enum comparison_reason reason)
{
    enum type_initial_check_result ret = TYPES_ARE_NOT_EQUAL;

    if (from->category == to->category) {
        ret = TYPES_CHECK_CAN_CONTINUE;
    }

    // A type can be compared to a leaf of the same type and to
    // a defined of the same type but should not be considered identical to it
    if (reason != TYPECMP_IDENTICAL) {
        if (from->category == TYPE_CATEGORY_ELEMENTARY && to->category == TYPE_CATEGORY_LEAF) {
            if (type_same_categories_compare(from, to->leaf.type, reason)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (to->category == TYPE_CATEGORY_ELEMENTARY && from->category == TYPE_CATEGORY_LEAF) {
            if (type_same_categories_compare(from->leaf.type, to, reason)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (from->category == TYPE_CATEGORY_DEFINED) {
            if (type_compare(from->defined.type, to, reason)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (to->category == TYPE_CATEGORY_DEFINED) {
            if (type_compare(from, to->defined.type, reason)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (to->category == TYPE_CATEGORY_OPERATOR &&
                   RF_BITFLAG_ON(g_typecmp_ctx.flags, TYPECMP_FLAG_FUNCTION_CALL)) {
            if (type_compare_to_operator(from, &to->operator, reason)) {
                ret = TYPES_ARE_EQUAL;
            }
		}
    }

    return ret;
}

bool type_compare(const struct type *from,
                  const struct type *to,
                  enum comparison_reason reason)
{
    typecmp_ctx_reset();
    // first check if we refer to the same type (elementary or composite)
    if (from == to) {
        TYPECMP_RETURN(true);
    }

    switch (type_category_check(from, to, reason)) {
    case TYPES_ARE_EQUAL:
        return true;
    case TYPES_ARE_NOT_EQUAL:
        TYPECMP_RETURN(false);
    case TYPES_CHECK_CAN_CONTINUE:
        break;
    }

    // from here and on we must have same categories of types
    return type_same_categories_compare(from, to, reason);
}

bool type_equals_ast_node(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl)
{
    struct type *looked_up_t;
    switch(type_desc->type) {
    case AST_TYPE_OPERATOR:
        if (t->category != TYPE_CATEGORY_OPERATOR) {
            return false;
        }

        if (t->operator.type != ast_typeop_op(type_desc)) {
            return false;
        }

        return type_equals_ast_node(t->operator.left,
                                    ast_typeop_left(type_desc),
                                    a, st, genrdecl) &&
            type_equals_ast_node(t->operator.right,
                                 ast_typeop_right(type_desc),
                                 a, st, genrdecl);
    case AST_TYPE_DESCRIPTION:
    {
        AST_NODE_ASSERT_TYPE(ast_typedesc_left(type_desc), AST_IDENTIFIER);
        bool predicate = true;
        if (t->category == TYPE_CATEGORY_LEAF) {
            predicate = rf_string_equal(t->leaf.id, ast_identifier_str(ast_typedesc_left(type_desc)));
        }
        return predicate && type_equals_ast_node(t, ast_typedesc_right(type_desc),
                                                 a, st, genrdecl);
    }
    case AST_TYPE_DECLARATION:
        return type_equals_ast_node(t, ast_typedecl_typedesc_get(type_desc),
                                    a, st, genrdecl);
    case AST_XIDENTIFIER:
        looked_up_t = type_lookup_xidentifier(type_desc, a, st, genrdecl);
        if (!looked_up_t) {
            RF_ERROR("Failed to lookup an identifier");
            return false;
        }

        return type_compare(t, looked_up_t, TYPECMP_GENERIC);
    default:
        break;
    }

    RF_ASSERT_OR_CRITICAL(false, "Illegal ast node type \""RF_STR_PF_FMT"\""
                          " detected instead of a type description",
                          RF_STR_PF_ARG(ast_node_str(type_desc)));
    return false;
}
