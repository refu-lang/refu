#include <types/type_elementary.h>

#include <Utils/bits.h>         // for RF_BITFLAG_SET
#include <Utils/build_assert.h> // for BUILD_ASSERT
#include <String/rf_str_core.h> // for RF_STRING_STATIC_INIT

#include <types/type.h>

#include "elementary_types_htable.h"

// NOTE: preserve order
static const struct RFstring elementary_type_strings[] = {
    RF_STRING_STATIC_INIT("int"),
    RF_STRING_STATIC_INIT("uint"),
    RF_STRING_STATIC_INIT("i8"),
    RF_STRING_STATIC_INIT("u8"),
    RF_STRING_STATIC_INIT("i16"),
    RF_STRING_STATIC_INIT("u16"),
    RF_STRING_STATIC_INIT("i32"),
    RF_STRING_STATIC_INIT("u32"),
    RF_STRING_STATIC_INIT("i64"),
    RF_STRING_STATIC_INIT("u64"),
    RF_STRING_STATIC_INIT("f32"),
    RF_STRING_STATIC_INIT("f64"),
    RF_STRING_STATIC_INIT("string"),
    RF_STRING_STATIC_INIT("bool"),
    RF_STRING_STATIC_INIT("nil")
};

// NOTE: preserve order
static struct type i_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)                           \
    [i_type] = {.category = TYPE_CATEGORY_ELEMENTARY, .elementary = {.etype=i_type}}

    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};

static inline bool type_elementary_is_int(const struct type_elementary *t)
{
    return t->etype <= ELEMENTARY_TYPE_UINT_64;
}

static inline bool type_elementary_int_is_unsigned(const struct type_elementary *t)
{
    return t->etype % 2 != 0;
}

static inline bool type_elementary_is_unsigned(const struct type_elementary *t)
{
    return type_elementary_is_int(t) && type_elementary_int_is_unsigned(t);
}

static inline bool type_elementary_is_float(const struct type_elementary *t)
{
    return t->etype >= ELEMENTARY_TYPE_FLOAT_32 && t->etype <= ELEMENTARY_TYPE_FLOAT_64;
}

bool type_elementary_equals(const struct type_elementary *t1,
                            const struct type_elementary *t2,
                            struct type_comparison_ctx *ctx)
{
    if (t1->etype == t2->etype) {
        if (ctx) {
            ctx->common_type = type_elementary_get_type(t1->etype);
        }
        return true;
    }

    if (!ctx) { // TODO: Is this really what we want here???
        return false;
    }

    if (type_comparison_ctx_reason(ctx) == COMPARISON_REASON_IDENTICAL) {
        return false;
    }

    if (t1->etype <= ELEMENTARY_TYPE_UINT_64 &&
        t2->etype <= ELEMENTARY_TYPE_UINT_64) {
        struct type *largest_type = type_elementary_get_type(t1->etype);;

        if (type_elementary_int_is_unsigned(t1) &&
            !type_elementary_is_unsigned(t2)) {

            RF_BITFLAG_SET(ctx->conversion, SIGNED_TO_UNSIGNED);
        }

        if (t1->etype < t2->etype) {
            if (ctx->reason == COMPARISON_REASON_ASSIGNMENT) {
                // it's an error to assign bigger type to smaller type
                return false;
            }
            // else
            RF_BITFLAG_SET(ctx->conversion, LARGER_TO_SMALLER);
            largest_type = type_elementary_get_type(t2->etype);
        }

        ctx->common_type = largest_type;
        return true;
    }

    if ((type_elementary_is_float(t1) && type_elementary_is_int((t2))) ||
        (type_elementary_is_float(t2) && type_elementary_is_int((t1)))) {
        // some operations between float and ints are allowed for now
        // except for:
        if (ctx->reason == COMPARISON_REASON_ASSIGNMENT) {
            return false;
        }

        //TODO: warn/disallow implicit conversions to float here
        ctx->common_type = type_elementary_get_type(ELEMENTARY_TYPE_FLOAT_64);
        return true;
    }

    if (t1->etype >= ELEMENTARY_TYPE_FLOAT_32 && t1->etype <= ELEMENTARY_TYPE_FLOAT_64 &&
        t2->etype >= ELEMENTARY_TYPE_FLOAT_32 && t2->etype <= ELEMENTARY_TYPE_FLOAT_64) {
        ctx->common_type = type_elementary_get_type(ELEMENTARY_TYPE_FLOAT_64);
        // they are both floats, we can work with it
        return true;
    }

    return false;
}

struct type *type_elementary_get_type(enum elementary_type etype)
{
    return &i_elementary_types[etype];
}

const struct RFstring *type_elementary_get_str(enum elementary_type etype)
{
    return &elementary_type_strings[etype];
}

int type_elementary_identifier_p(const struct RFstring *id)
{
    const struct gperf_elementary_type *etype;
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(elementary_type_strings)/sizeof(struct RFstring) == ELEMENTARY_TYPE_TYPES_COUNT
    );

    etype = types_string_is_elementary(rf_string_data(id),
                                    rf_string_length_bytes(id));

    if (!etype) {
        return -1;
    }

    return etype->type;
}

enum elementary_type_category type_elementary_get_category(const struct type *t)
{
    if (t->category != TYPE_CATEGORY_ELEMENTARY) {
        return -1;
    }
    if (t->elementary.etype < 10) {
        if (t->elementary.etype % 2 == 0) {
            return ELEMENTARY_TYPE_CATEGORY_SIGNED;
        }
        // else
        return ELEMENTARY_TYPE_CATEGORY_UNSIGNED;
    } else if (t->elementary.etype < 12) {
        return ELEMENTARY_TYPE_CATEGORY_FLOAT;
    }
    return ELEMENTARY_TYPE_CATEGORY_OTHER;
}

i_INLINE_INS bool type_is_specific_elementary(const struct type *t, enum elementary_type etype);
i_INLINE_INS bool type_is_simple_elementary(const struct type *t);
i_INLINE_INS enum elementary_type type_elementary(const struct type *t);
i_INLINE_INS bool type_is_signed_elementary(const struct type *t);
i_INLINE_INS bool type_is_unsigned_elementary(const struct type *t);
i_INLINE_INS bool type_is_floating_elementary(const struct type *t);
