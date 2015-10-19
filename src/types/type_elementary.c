#include <types/type_elementary.h>

#include <Utils/bits.h>         // for RF_BITFLAG_SET
#include <Utils/build_assert.h> // for BUILD_ASSERT
#include <String/rf_str_core.h> // for RF_STRING_STATIC_INIT

#include <types/type.h>
#include <types/type_comparisons.h>

#include "elementary_types_htable.h"

// NOTE: preserve order
static const struct RFstring elementary_type_strings[] = {
    RF_STRING_STATIC_INIT("i8"),
    RF_STRING_STATIC_INIT("u8"),
    RF_STRING_STATIC_INIT("i16"),
    RF_STRING_STATIC_INIT("u16"),
    RF_STRING_STATIC_INIT("i32"),
    RF_STRING_STATIC_INIT("u32"),
    RF_STRING_STATIC_INIT("i64"),
    RF_STRING_STATIC_INIT("u64"),
    RF_STRING_STATIC_INIT("int"),
    RF_STRING_STATIC_INIT("uint"),
    RF_STRING_STATIC_INIT("f32"),
    RF_STRING_STATIC_INIT("f64"),
    RF_STRING_STATIC_INIT("string"),
    RF_STRING_STATIC_INIT("bool"),
    RF_STRING_STATIC_INIT("nil")
};

// NOTE: preserve order
static struct type i_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)    \
    [i_type] = {                                    \
        .category = TYPE_CATEGORY_ELEMENTARY,       \
        .elementary = {                             \
            .etype=i_type,                          \
            .is_constant = false                    \
        }                                           \
    }
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};
static struct type i_elementary_types_constant[] = {    
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(i_type)   \
    [i_type] = {                                    \
        .category = TYPE_CATEGORY_ELEMENTARY,       \
        .elementary = {                             \
            .etype=i_type,                          \
            .is_constant = true                     \
        }                                           \
    }
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX2(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX2
};

struct type *type_elementary_get_type(enum elementary_type etype)
{
    return &i_elementary_types[etype];
}

struct type *type_elementary_get_type_constant(enum elementary_type etype)
{
    return &i_elementary_types_constant[etype];
}

const struct RFstring *type_elementary_get_str(enum elementary_type etype)
{
    return &elementary_type_strings[etype];
}

enum elementary_type type_elementary_from_str(const struct RFstring *s)
{
    //TODO: perhaps having a hash table here would be better. Reuse the one from gperf?
    unsigned int i = 0;
    for (i = 0; i < sizeof(elementary_type_strings) / sizeof(struct RFstring); ++i) {
        if (rf_string_equal(s, &elementary_type_strings[i])) {
            return (enum elementary_type)i;
        }
    }
    return ELEMENTARY_TYPE_TYPES_COUNT;
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
    if (t->elementary.etype <= ELEMENTARY_TYPE_UINT) {
        if (t->elementary.etype % 2 == 0) {
            return ELEMENTARY_TYPE_CATEGORY_SIGNED;
        }
        // else
        return ELEMENTARY_TYPE_CATEGORY_UNSIGNED;
    } else if (t->elementary.etype < ELEMENTARY_TYPE_STRING) {
        return ELEMENTARY_TYPE_CATEGORY_FLOAT;
    }
    return ELEMENTARY_TYPE_CATEGORY_OTHER;
}

i_INLINE_INS bool elementary_type_is_int(enum elementary_type etype);
i_INLINE_INS bool type_elementary_has_explicit_conversion(const struct type_elementary *t);
i_INLINE_INS bool type_elementary_int_is_unsigned(const struct type_elementary *t);
i_INLINE_INS bool type_elementary_is_unsigned(const struct type_elementary *t);
i_INLINE_INS bool type_elementary_is_signed(const struct type_elementary *t);
i_INLINE_INS bool elementary_type_is_float(enum elementary_type etype);
i_INLINE_INS bool elementary_type_is_numeric(enum elementary_type etype);
i_INLINE_INS int elementary_type_to_bytesize(enum elementary_type etype);
i_INLINE_INS int type_elementary_bytesize(const struct type_elementary *t);
i_INLINE_INS bool type_is_elementary(const struct type *t);
i_INLINE_INS bool type_is_specific_elementary(const struct type *t, enum elementary_type etype);
i_INLINE_INS bool type_is_simple_elementary(const struct type *t);
i_INLINE_INS bool type_is_numeric_elementary(const struct type *t);
i_INLINE_INS enum elementary_type type_elementary(const struct type *t);
i_INLINE_INS bool type_is_signed_elementary(const struct type *t);
i_INLINE_INS bool type_is_unsigned_elementary(const struct type *t);
i_INLINE_INS bool type_is_floating_elementary(const struct type *t);
i_INLINE_INS bool type_is_explicitly_convertable_elementary(const struct type *t);
i_INLINE_INS bool type_is_constant_elementary(const struct type *t);
