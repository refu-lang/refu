#include <types/type_builtin.h>

#include <Utils/bits.h>         // for RF_BITFLAG_SET
#include <Utils/build_assert.h> // for BUILD_ASSERT
#include <String/rf_str_core.h> // for RF_STRING_STATIC_INIT

#include "builtin_types_htable.h"

// NOTE: preserve order
static const struct RFstring builtin_type_strings[] = {
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
};

// NOTE: preserve order
static const struct type i_builtin_types[] = {
#define INIT_BUILTIN_TYPE_ARRAY_INDEX(i_type)                           \
    [i_type] = {.category = TYPE_CATEGORY_BUILTIN, .builtin = {.btype=i_type}}

    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_INT),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_UINT),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_INT_8),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_UINT_8),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_INT_16),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_UINT_16),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_INT_32),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_UINT_32),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_INT_64),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_UINT_64),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_FLOAT_32),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_FLOAT_64),
    INIT_BUILTIN_TYPE_ARRAY_INDEX(BUILTIN_STRING)
#undef INIT_BUILTIN_TYPE_ARRAY_INDEX
};

static inline bool type_builtin_int_is_unsigned(const struct type_builtin *t)
{
    return t->btype % 2 != 0;
}

static inline bool type_builtin_is_unsigned(const struct type_builtin *t)
{
    return t->btype <= BUILTIN_UINT_64 && type_builtin_int_is_unsigned(t);
}

bool type_builtin_equals(const struct type_builtin *t1,
                         const struct type_builtin *t2,
                         struct type_comparison_ctx *ctx)
{
    if (t1->btype == t2->btype) {
        return true;
    }

    if (!ctx) {
        return false;
    }

    if (t1->btype <= BUILTIN_UINT_64 &&
        t2->btype <= BUILTIN_UINT_64) {

        if (ctx->reason == COMPARISON_REASON_ASSIGNMENT) {
            if (type_builtin_int_is_unsigned(t1) &&
                !type_builtin_is_unsigned(t2)) {

                RF_BITFLAG_SET(ctx->conversion, SIGNED_TO_UNSIGNED);
            }

            if (t1->btype < t2->btype) {
                RF_BITFLAG_SET(ctx->conversion, LARGER_TO_SMALLER);
            }
        }
        return true;
    }


    if (t1->btype >= BUILTIN_FLOAT_32 && t1->btype <= BUILTIN_FLOAT_64 &&
        t2->btype >= BUILTIN_FLOAT_32 && t2->btype <= BUILTIN_FLOAT_64) {
        // they are both floats, we can work with it
        return true;
    }

    return false;
}

const struct type *type_builtin_get_type(enum builtin_type btype)
{
    return &i_builtin_types[btype];
}

const struct RFstring *type_builtin_get_str(enum builtin_type btype)
{
    return &builtin_type_strings[btype];
}

int type_builtin_identifier_p(const struct RFstring *id)
{
    const struct gperf_builtin_type *btype;
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(builtin_type_strings)/sizeof(struct RFstring) == BUILTIN_TYPES_COUNT
    );

    btype = types_string_is_builtin(rf_string_data(id),
                                    rf_string_length_bytes(id));

    if (!btype) {
        return -1;
    }

    return btype->type;
}

i_INLINE_INS enum builtin_type type_builtin(struct type *t);
