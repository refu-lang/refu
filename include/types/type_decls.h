#ifndef LFR_TYPES_DECL_H
#define LFR_TYPES_DECL_H

#include <stdbool.h>
#include <String/rf_str_decl.h>
#include <ast/type_decls.h> // for enum type_op
#include <ast/operators_decls.h> // for binary operations enum
#include <Data_Structures/intrusive_list.h>

// NOTE: preserve order
enum builtin_type {
    BUILTIN_INT = 0,
    BUILTIN_UINT,
    BUILTIN_INT_8,
    BUILTIN_UINT_8,
    BUILTIN_INT_16,
    BUILTIN_UINT_16,
    BUILTIN_INT_32,
    BUILTIN_UINT_32,
    BUILTIN_INT_64,
    BUILTIN_UINT_64,
    BUILTIN_FLOAT_32,
    BUILTIN_FLOAT_64,
    BUILTIN_STRING,

    BUILTIN_TYPES_COUNT /* keep as last */
};


enum type_category {
    TYPE_CATEGORY_ANONYMOUS = 0,        /* an anonymous type */
    TYPE_CATEGORY_LEAF,                 /* almost always part of another type */
    TYPE_CATEGORY_BUILTIN,              /* a builtin type */
    TYPE_CATEGORY_USER_DEFINED,         /* a user defined type */
    TYPE_CATEGORY_GENERIC,              /* a generic type as declared by the user */
    TYPE_CATEGORY_FUNCTION,             /* a function */
};

struct type_builtin {
    enum builtin_type btype;
};

struct type_generic {
    const struct RFstring id;
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

struct type_composite {
    bool is_operator;
    union {
        struct type_operator op;
        struct type_leaf leaf;
    };
};

struct type_defined {
    const struct RFstring *id;
    struct type_composite *type;
};

struct type_function {
    struct type *argument_type;
    struct type *return_type;
};

struct type {
    enum type_category category;
    /* list handler, to be added to either types or anonymous types list */
    struct RFilist_node lh;
    union {
        struct type_builtin builtin;
        struct type_defined defined;
        struct type_function function;
        struct type_composite anonymous;
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
