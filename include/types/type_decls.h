#ifndef LFR_TYPES_DECL_H
#define LFR_TYPES_DECL_H

#include <stdbool.h>
#include <String/rf_str_decl.h>
#include <ast/type_decls.h> // for enum type_op
#include <ast/operators_decls.h> // for binary operations enum
#include <Data_Structures/intrusive_list.h>

// NOTE: preserve order
enum elementary_type {
    ELEMENTARY_TYPE_INT = 0,
    ELEMENTARY_TYPE_UINT,
    ELEMENTARY_TYPE_INT_8,
    ELEMENTARY_TYPE_UINT_8,
    ELEMENTARY_TYPE_INT_16,
    ELEMENTARY_TYPE_UINT_16,
    ELEMENTARY_TYPE_INT_32,
    ELEMENTARY_TYPE_UINT_32,
    ELEMENTARY_TYPE_INT_64,
    ELEMENTARY_TYPE_UINT_64,
    ELEMENTARY_TYPE_FLOAT_32,
    ELEMENTARY_TYPE_FLOAT_64,
    ELEMENTARY_TYPE_STRING,
    ELEMENTARY_TYPE_BOOL,

    ELEMENTARY_TYPE_TYPES_COUNT /* keep as last */
};


enum type_category {
    TYPE_CATEGORY_OPERATOR = 0,         /* a type combination of other types */
    TYPE_CATEGORY_LEAF,                 /* almost always part of another type */
    TYPE_CATEGORY_ELEMENTARY,           /* an elementary/builtin type */
    TYPE_CATEGORY_GENERIC,              /* a generic type as declared by the user */
};

struct type_elementary {
    enum elementary_type etype;
};

struct type_leaf {
    const struct RFstring *id;
    struct type *type;
};

struct type_operator {
    enum typeop_type type;
    struct type *left;
    struct type *right;
};

struct type {
    enum type_category category;
    /* list handler, to be added to either the types or the composite types list */
    struct RFilist_node lh;
    union {
        struct type_elementary elementary;
        struct type_operator operator;
        struct type_leaf leaf;
    };
};

/* -- type conversion declarations -- */
enum conversion_type {
    NO_CONVERSION = 0x0,
    SIGNED_TO_UNSIGNED = 0x1,
    LARGER_TO_SMALLER = 0X2,
};

enum comparison_reason {
    COMPARISON_REASON_ASSIGNMENT = BINARYOP_ASSIGN,
    COMPARISON_REASON_ADDITION = BINARYOP_ADD,
    COMPARISON_REASON_SUBTRACTION = BINARYOP_SUB,
    COMPARISON_REASON_MULTIPLICATION = BINARYOP_MUL,
    COMPARISON_REASON_DIVISION = BINARYOP_DIV,

    COMPARISON_REASON_FUNCTION_CALL,
};

struct type_comparison_ctx {
    //! The reason for the request of
    enum comparison_reason reason;
    //! Query to see what conversions happened. Can contain multiple bitflags
    enum conversion_type conversion;
    //! If any conversion happened this should point to the converted type
    struct type *converted_type;
};
#endif
