#include <types/type_elementary.h>

#include <Utils/bits.h>         // for RF_BITFLAG_SET
#include <Utils/build_assert.h> // for BUILD_ASSERT
#include <String/rf_str_core.h> // for RF_STRING_STATIC_INIT

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
};

// NOTE: preserve order
static const struct type i_elementary_types[] = {
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
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL)
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
        return true;
    }

    if (!ctx) {
        return false;
    }

    if (t1->etype <= ELEMENTARY_TYPE_UINT_64 &&
        t2->etype <= ELEMENTARY_TYPE_UINT_64) {


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
        }
        return true;
    }

    if ((type_elementary_is_float(t1) && type_elementary_is_int((t2))) ||
        (type_elementary_is_float(t2) && type_elementary_is_int((t1)))) {
        // operations between float and ints are allowed

        //TODO: Warn/disallow implicit conversions here
        return true;
    }

    if (t1->etype >= ELEMENTARY_TYPE_FLOAT_32 && t1->etype <= ELEMENTARY_TYPE_FLOAT_64 &&
        t2->etype >= ELEMENTARY_TYPE_FLOAT_32 && t2->etype <= ELEMENTARY_TYPE_FLOAT_64) {
        // they are both floats, we can work with it
        return true;
    }

    return false;
}

const struct type *type_elementary_get_type(enum elementary_type etype)
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

i_INLINE_INS enum elementary_type type_elementary(struct type *t);
