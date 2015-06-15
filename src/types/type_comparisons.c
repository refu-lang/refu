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
    bool conversion_at_final_match;
    const struct type *matched_type;
    struct RFstringx err_buff;
    struct RFstringx warn_buff;
    int count;
    struct {darray(struct RFstring);} warning_indices;
};

i_THREAD__ struct typecmp_ctx g_typecmp_ctx;

#define TYPECMP_RETURN(i_retvalue_)             \
    g_typecmp_ctx.needs_reset = true;           \
    return i_retvalue_

// just like TYPECMP_RETURN(true) but also set the matched type properly
#define TYPECMP_RETSET_SUCCESS(i_matched_)              \
    g_typecmp_ctx.matched_type = i_matched_;            \
    g_typecmp_ctx.needs_reset = true;                   \
    g_typecmp_ctx.conversion_at_final_match = false;    \
    return true

// just like TYPECMP_RETSET_SUCCESS() but also specify that success occured due to conversion
#define TYPECMP_RETSET_SUCCESS_CONVERSION(i_matched_)   \
    g_typecmp_ctx.matched_type = i_matched_;            \
    g_typecmp_ctx.conversion_at_final_match = true;     \
    return true

bool typecmp_ctx_init()
{
    darray_init(g_typecmp_ctx.warning_indices);
    g_typecmp_ctx.count = 0;
    g_typecmp_ctx.needs_reset = false;
    g_typecmp_ctx.conversion_at_final_match = false;
    g_typecmp_ctx.flags = 0;
    g_typecmp_ctx.matched_type = NULL;
    return rf_stringx_init_buff(&g_typecmp_ctx.err_buff, 1024, "") &&
        rf_stringx_init_buff(&g_typecmp_ctx.warn_buff, 1024, "");
}

static inline void typecmp_ctx_reset()
{
    if (g_typecmp_ctx.needs_reset && g_typecmp_ctx.count <= 0) {
        g_typecmp_ctx.count = 0;
        g_typecmp_ctx.needs_reset = false;
        g_typecmp_ctx.matched_type = NULL;
        g_typecmp_ctx.conversion_at_final_match = false;
        g_typecmp_ctx.flags = 0;
        rf_stringx_assignv(&g_typecmp_ctx.err_buff, "");
        rf_stringx_assignv(&g_typecmp_ctx.warn_buff, "");
        while (g_typecmp_ctx.warning_indices.size != 0) {
            (void)darray_pop(g_typecmp_ctx.warning_indices);
        }
    }
    g_typecmp_ctx.count +=1;
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

const struct type *typemp_ctx_get_matched_type()
{
    return g_typecmp_ctx.matched_type;
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

static bool type_operator_compare(const struct type *from,
                                  const struct type *to,
                                  enum comparison_reason reason)
{
    if (from == to) {
        TYPECMP_RETSET_SUCCESS(from);
    }

    if (from->operator.type != to->operator.type) {
        TYPECMP_RETURN(false);
    }

    return type_compare(from->operator.left, to->operator.left, reason) &&
        type_compare(from->operator.right, to->operator.right, reason);
}



static bool type_elementary_compare(const struct type *fromtype,
                                    const struct type *totype,
                                    enum comparison_reason reason)
{
    // keep the enum and the array synced
    enum error_explanation_indices {
        TYPECMP_ERRXPLAIN_NONE = 0,
        TYPECMP_ERRXPLAIN_SECOND_IMPLICIT,
        TYPECMP_ERRXPLAIN_LARGESMALL_CONSTANT,
        TYPECMP_ERRXPLAIN_SIGNEDUNSIGNED_CONSTANT,
    } current_error_type = 0;
    static const char *error_explanations[] = {
        "",
        ". An implicit conversion already happened",
        ". Attempting to assign larger constant to smaller variable",
        ". Attempting to assign signed constant to unsigned variable",
    };
    const struct type_elementary *from = &fromtype->elementary;
    const struct type_elementary *to = &totype->elementary;
    if (from->etype == to->etype) {
        TYPECMP_RETSET_SUCCESS(fromtype);
    }

    // for identical comparison no conversions happen here
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
            // implicit conversion from bigger to smaller type is allowed also in pattern marching
            if (type_elementary_bytesize(from) > type_elementary_bytesize(to)) {

                if (from->is_constant) {
                    // error
                    current_error_type = TYPECMP_ERRXPLAIN_LARGESMALL_CONSTANT;
                    goto end;
                }
                if (reason != TYPECMP_EXPLICIT_CONVERSION) {
                    // warning
                    RFS_PUSH();
					typecmp_ctx_add_warning(
                        RFS_OR_DIE(
                            "Implicit conversion from \""RF_STR_PF_FMT"\" to \""
                            RF_STR_PF_FMT"\"",
                            RF_STR_PF_ARG(type_elementary_get_str(from->etype)),
                            RF_STR_PF_ARG(type_elementary_get_str(to->etype)))
                    );
                    RFS_POP();
                }
            }
            // implicit conversion from signed to unsigned allowed but not during pattern matching
            if (!type_elementary_int_is_unsigned(from) && type_elementary_int_is_unsigned(to)) {
                if (from->is_constant || reason == TYPECMP_PATTERN_MATCHING) {
                    // error
                    current_error_type = TYPECMP_ERRXPLAIN_SIGNEDUNSIGNED_CONSTANT;
                    goto end;
                }
                
                if (reason != TYPECMP_EXPLICIT_CONVERSION) {
                    // warning
                    RFS_PUSH();
					typecmp_ctx_add_warning(
                        RFS_OR_DIE(
                            "Implicit signed to unsigned conversion from \""RF_STR_PF_FMT"\" "
                            "to \""RF_STR_PF_FMT"\"",
                            RF_STR_PF_ARG(type_elementary_get_str(from->etype)),
                            RF_STR_PF_ARG(type_elementary_get_str(to->etype)))
                    );
                    RFS_POP();
                }
            }
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // no other implicit conversions allowed in patter matching
        if (reason == TYPECMP_PATTERN_MATCHING) {
            goto end;
        }

        // an int is implicitly convertible to a float
        if (type_elementary_is_float(to)) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // an int is implicitly convertible to a bool
        if (to->etype == ELEMENTARY_TYPE_BOOL) { // we can convert from int to bool
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // explicit conversion from int constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            from->is_constant &&
            reason == TYPECMP_EXPLICIT_CONVERSION) {
            
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }
        break;
    case ELEMENTARY_TYPE_FLOAT_32:
    case ELEMENTARY_TYPE_FLOAT_64:
        // float to float is okay
        if (type_elementary_is_float(to)) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // only explicit float to int
        if (type_elementary_is_int(to) && reason == TYPECMP_EXPLICIT_CONVERSION) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // explicit conversion from float constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            from->is_constant &&
            reason == TYPECMP_EXPLICIT_CONVERSION) {
            
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }
        break;
    case ELEMENTARY_TYPE_BOOL:
        // bool is not implicitly convertable during pattern matching
        if (reason == TYPECMP_PATTERN_MATCHING) {
            break;
        }
        if (reason == TYPECMP_AFTER_IMPLICIT_CONVERSION) {
            // if we only had an implicit conversion bool is not convertable
            current_error_type = TYPECMP_ERRXPLAIN_SECOND_IMPLICIT;
            break;
        }
        if (type_elementary_is_int(to)) { // we can convert from bool to int
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }
        // we can explicitly convert from bool to string
        if (to->etype == ELEMENTARY_TYPE_STRING && reason == TYPECMP_EXPLICIT_CONVERSION) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
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
                       error_explanations[current_error_type]
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
        return type_operator_compare(from, to, reason);
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_compare(from, to, reason);
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
        // if we are during pattern matching persist the reason, else state
        // that one implicit conversion already happened (due to taking
        // one side of a sum operator)
        enum comparison_reason new_reason = reason == TYPECMP_PATTERN_MATCHING
            ? TYPECMP_PATTERN_MATCHING : TYPECMP_AFTER_IMPLICIT_CONVERSION;

        bool left_result = type_compare(from, to->left, new_reason);
        if (left_result && !g_typecmp_ctx.conversion_at_final_match) {
            // had an exact match
            return true;
        }

        bool right_result = type_compare(from, to->right, new_reason);
        if (right_result && !g_typecmp_ctx.conversion_at_final_match) {
            // had an exact match
            return true;
        }
        // no exact match, but still check if there was a conversion match
        return left_result || right_result;
    }
    return false;
}

//! Possible resuls of @see type_initial_check()
enum type_initial_check_result {
    TYPES_ARE_EQUAL = 2,
    TYPES_ARE_CONVERTABLE = 1,
    TYPES_CHECK_CAN_CONTINUE = 0,
    TYPES_ARE_NOT_EQUAL = -1,
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

    if (reason == TYPECMP_PATTERN_MATCHING) {
        // wildmark equals everything
        if (from->category == TYPE_CATEGORY_WILDCARD && to->category != TYPE_CATEGORY_OPERATOR) {
            return TYPES_ARE_EQUAL;
        }
        // if match type is a defined type (as should be in most cases), compare to definition
        if (to->category == TYPE_CATEGORY_DEFINED) {
            return type_compare(from, to->defined.type, reason) ?
                TYPES_ARE_EQUAL : TYPES_ARE_NOT_EQUAL;
        }
    }
    if (reason != TYPECMP_IDENTICAL) {
        // A type can be compared to a leaf of the same type and to
        // a defined of the same type but should not be considered identical to it
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
                   (RF_BITFLAG_ON(g_typecmp_ctx.flags, TYPECMP_FLAG_FUNCTION_CALL) ||
                    reason == TYPECMP_PATTERN_MATCHING)) {
            if (type_compare_to_operator(from, &to->operator, reason)) {
                ret = g_typecmp_ctx.conversion_at_final_match ? TYPES_ARE_EQUAL : TYPES_ARE_CONVERTABLE;
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
        TYPECMP_RETSET_SUCCESS(to);
    }

    switch (type_category_check(from, to, reason)) {
    case TYPES_ARE_EQUAL:
        g_typecmp_ctx.count -=1;
        TYPECMP_RETURN(true);
    case TYPES_ARE_CONVERTABLE:
        g_typecmp_ctx.count +=1;
        return true; //without resetting
    case TYPES_ARE_NOT_EQUAL:
        g_typecmp_ctx.count +=1;
        TYPECMP_RETURN(false);
    case TYPES_CHECK_CAN_CONTINUE:
        break;
    }

    // from here and on we must have same categories of types
    bool ret = type_same_categories_compare(from, to, reason);
    g_typecmp_ctx.count +=1;
    return ret;
}

bool type_equals_ast_node(struct type *t,
                          const struct ast_node *type_desc,
                          struct analyzer *a,
                          struct symbol_table *st,
                          struct ast_node *genrdecl,
                          enum comparison_reason options)
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
                                    a, st, genrdecl, options) &&
            type_equals_ast_node(t->operator.right,
                                 ast_typeop_right(type_desc),
                                 a, st, genrdecl, options);
    case AST_TYPE_LEAF:
    {
        AST_NODE_ASSERT_TYPE(ast_typeleaf_left(type_desc), AST_IDENTIFIER);
        if (t->category != TYPE_CATEGORY_LEAF) {
            return false;
        }
        return rf_string_equal(t->leaf.id, ast_identifier_str(ast_typeleaf_left(type_desc))) &&
            type_equals_ast_node(
                t,
                ast_typeleaf_right(type_desc),
                a,
                st,
                genrdecl,
                TYPECMP_IDENTICAL
            );
    }
    case AST_TYPE_DESCRIPTION:
        return type_equals_ast_node(t, ast_typedesc_desc_get(type_desc), a, st, genrdecl, options);
    case AST_TYPE_DECLARATION:
        return type_equals_ast_node(
            t,
            ast_typedecl_typedesc_get(type_desc),
            a,
            st,
            genrdecl,
            options
        );
    case AST_XIDENTIFIER:
        looked_up_t = type_lookup_xidentifier(type_desc, a, st, genrdecl);
        if (!looked_up_t) {
            RF_ERROR("Failed to lookup an identifier");
            return false;
        }

        // if we get here due to a comparison with a typeleaf then the left
        // identifier must have matched. Only type remains to be matched.
        if (t->category == TYPE_CATEGORY_LEAF) {
            t = t->leaf.type;
        }

        return type_compare(t, looked_up_t, options);
    default:
        break;
    }

    RF_ASSERT_OR_CRITICAL(false, return false,
                          "Illegal ast node type \""RF_STR_PF_FMT"\""
                          " detected instead of a type description",
                          RF_STR_PF_ARG(ast_node_str(type_desc)));
}
