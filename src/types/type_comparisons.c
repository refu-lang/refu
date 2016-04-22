#include <types/type_comparisons.h>

#include <rflib/utils/sanity.h>
#include <rflib/utils/bits.h>
#include <rflib/defs/threadspecific.h>
#include <rflib/string/common.h>
#include <rflib/string/core.h>
#include <rflib/string/corex.h>
#include <rflib/string/conversion.h>
#include <rflib/string/manipulationx.h>
#include <rflib/datastructs/darray.h>

#include <ast/ast.h>
#include <ast/type.h>

#include <types/type.h>
#include <types/type_operators.h>
#include <types/type_arr.h>
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

    if (type_get_subtypes_num(from) != type_get_subtypes_num(to)) {
        TYPECMP_RETURN(false);
    }

    struct type **subt;
    unsigned int idx = 0;
    darray_foreach(subt, from->operator.operands) {
        if (!type_compare(*subt, darray_item(to->operator.operands, idx), reason)) {
            return false;
        }
        ++idx;
    }
    return true;
}



static bool type_elementary_compare(
    const struct type *fromtype,
    const struct type *totype,
    enum comparison_reason reason
)
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
        if (elementary_type_is_int(to->etype)) {
            // implicit conversion from bigger to smaller type is allowed also in pattern marching
            if (type_elementary_bytesize(from) > type_elementary_bytesize(to)) {

                if (fromtype->is_constant) {
                    // error
                    current_error_type = TYPECMP_ERRXPLAIN_LARGESMALL_CONSTANT;
                    goto end_error_msg;
                }
                if (reason != TYPECMP_EXPLICIT_CONVERSION) {
                    // warning
                    RFS_PUSH();
					typecmp_ctx_add_warning(
                        RFS_OR_DIE(
                            "Implicit conversion from \""RFS_PF"\" to \""
                            RFS_PF"\"",
                            RFS_PA(type_elementary_get_str(from->etype)),
                            RFS_PA(type_elementary_get_str(to->etype)))
                    );
                    RFS_POP();
                }
            }
            // implicit conversion from signed to unsigned allowed but not during pattern matching
            if (!type_elementary_int_is_unsigned(from) && type_elementary_int_is_unsigned(to)) {
                if (fromtype->is_constant || reason == TYPECMP_PATTERN_MATCHING) {
                    // error
                    current_error_type = TYPECMP_ERRXPLAIN_SIGNEDUNSIGNED_CONSTANT;
                    goto end_error_msg;
                }

                if (reason != TYPECMP_EXPLICIT_CONVERSION) {
                    // warning
                    RFS_PUSH();
					typecmp_ctx_add_warning(
                        RFS_OR_DIE(
                            "Implicit signed to unsigned conversion from \""
                            RFS_PF"\" ""to \""RFS_PF"\"",
                            RFS_PA(type_elementary_get_str(from->etype)),
                            RFS_PA(type_elementary_get_str(to->etype)))
                    );
                    RFS_POP();
                }
            }
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // no other implicit conversions allowed in pattern matching
        if (reason == TYPECMP_PATTERN_MATCHING) {
            goto end_error_msg;
        }

        // an int is implicitly convertible to a float
        if (elementary_type_is_float(to->etype)) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // an int is implicitly convertible to a bool
        if (to->etype == ELEMENTARY_TYPE_BOOL) { // we can convert from int to bool
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // explicit conversion from int constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            fromtype->is_constant &&
            reason == TYPECMP_EXPLICIT_CONVERSION) {

            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }
        break;
    case ELEMENTARY_TYPE_FLOAT_32:
    case ELEMENTARY_TYPE_FLOAT_64:
        // float to float is okay
        if (elementary_type_is_float(to->etype)) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // only explicit float to int
        if (elementary_type_is_int(to->etype) && reason == TYPECMP_EXPLICIT_CONVERSION) {
            TYPECMP_RETSET_SUCCESS_CONVERSION(totype);
        }

        // explicit conversion from float constant numeric literal to string is ok
        if (to->etype == ELEMENTARY_TYPE_STRING &&
            fromtype->is_constant &&
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
        if (elementary_type_is_int(to->etype)) { // we can convert from bool to int
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
        RF_CRITICAL_FAIL("Invalid elementary type at comparisons");
        break;
    }


end_error_msg:
    RFS_PUSH();
    rf_stringx_assignv(
        &g_typecmp_ctx.err_buff,
        "Unable to convert from \""RFS_PF"\" to \""RFS_PF"\"%s",
        RFS_PA(type_elementary_get_str(from->etype)),
        RFS_PA(type_elementary_get_str(to->etype)),
        error_explanations[current_error_type]
    );
    RFS_POP();
end:
    TYPECMP_RETURN(false);
}

static bool type_array_compare(
    const struct type *from,
    const struct type *to,
    enum comparison_reason reason)
{
    if (!type_compare(from->array.member_type, to->array.member_type, reason)) {
        RFS_PUSH();
        rf_stringx_assignv(
            &g_typecmp_ctx.err_buff,
            "Array member type mismatch. \""RFS_PF"\" != \""RFS_PF"\"",
            RFS_PA(type_str_or_die(from->array.member_type, TSTR_DEFAULT)),
            RFS_PA(type_str_or_die(to->array.member_type, TSTR_DEFAULT))
        );
        RFS_POP();
        TYPECMP_RETURN(false);
    }

    unsigned from_d = darray_size(from->array.dimensions);
    unsigned to_d = darray_size(to->array.dimensions);
    if (from_d != to_d) {
        rf_stringx_assignv(
            &g_typecmp_ctx.err_buff,
            "Array dimensions mismatch. %u != %u",
            from_d,
            to_d
        );
        TYPECMP_RETURN(false);
    }

    int64_t *fromval;
    unsigned i = 0;
    darray_foreach(fromval, from->array.dimensions) {
        int64_t toval = darray_item(to->array.dimensions, i++);
        if (*fromval != toval && *fromval != -1 && toval != -1) {
            RFS_PUSH();
            rf_stringx_assignv(
                &g_typecmp_ctx.err_buff,
                "Mismatch at the size of the "RFS_PF" array dimension "
                "%"PRId64" != %"PRId64"",
                RFS_PA(rf_string_ordinal(i)),
                *fromval,
                toval
            );
            RFS_POP();
            TYPECMP_RETURN(false);
        }
    }


    TYPECMP_RETURN(true);
}

static bool type_same_categories_compare(
    const struct type *from,
    const struct type *to,
    enum comparison_reason reason)
{
    RF_ASSERT(
        from->category == to->category,
        "type_same_categories_equals() called with different categories"
    );

    switch (from->category) {
    case TYPE_CATEGORY_OPERATOR:
        return type_operator_compare(from, to, reason);
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_compare(from, to, reason);
    case TYPE_CATEGORY_DEFINED:
        return rf_string_equal(from->defined.name, to->defined.name);
    case TYPE_CATEGORY_ARRAY:
        return type_array_compare(from, to, reason);
    case TYPE_CATEGORY_GENERIC:
        //TODO
        RF_CRITICAL_FAIL("Not yet implemented");
        break;
    default:
        RF_CRITICAL_FAIL("Illegal type category");
        break;
    }

    TYPECMP_RETURN(false);
}

// a special check for sum type operators. Other operators always return false
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
        struct type **subt;
        struct type *converted_type = NULL;
        darray_foreach(subt, to->operands) {
            if (type_compare(from, *subt, new_reason)) {
                if (g_typecmp_ctx.conversion_at_final_match) {
                    converted_type = *subt;
                } else {
                    // only if it is an exact match stop
                    TYPECMP_RETSET_SUCCESS(*subt);
                }
            }
        }
        // if we get here it means that no exact match happened but let's check
        // for a convertable type match
        if (converted_type) {
            // only if it is an exact match stop
            TYPECMP_RETSET_SUCCESS(converted_type);
        }
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
        if (from->category == TYPE_CATEGORY_DEFINED) {
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

struct ast_type_equality_ctx {
    struct module *mod;
    struct symbol_table *st;
    struct ast_node *genrdecl;
    enum comparison_reason options;
};

static inline void ast_type_equality_ctx_init(
    struct ast_type_equality_ctx *ctx,
    struct module *mod,
    struct symbol_table *st,
    struct ast_node *genrdecl,
    enum comparison_reason options
) {
    ctx->mod = mod;
    ctx->st = st;
    ctx->genrdecl = genrdecl;
    ctx->options = options;
}

static bool ast_type_equality_cb(
    const struct RFstring *name,
    const struct ast_node *desc,
    struct type *t,
    struct ast_type_equality_ctx *ctx)
{
    type_creation_ctx_set_args(ctx->mod, ctx->st, ctx->genrdecl);
    struct type *lookedup_t = type_lookup_xidentifier(desc);
    if (!lookedup_t) {
        RF_ERROR("Failed to lookup type of an identifier");
        return false;
    }
    return type_compare(t, lookedup_t, ctx->options);
}

bool type_equals_ast_node(struct type *t,
                          const struct ast_node *type_desc,
                          struct module *mod,
                          struct symbol_table *st,
                          struct ast_node *genrdecl,
                          enum comparison_reason options)
{
    struct ast_type_equality_ctx ctx;
    ast_type_equality_ctx_init(&ctx, mod, st, genrdecl, options);
    return ast_type_foreach_leaf_arg(type_desc, t, (ast_type_cb)ast_type_equality_cb, &ctx);
}
