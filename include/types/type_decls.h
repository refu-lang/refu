#ifndef LFR_TYPES_DECL_H
#define LFR_TYPES_DECL_H

#include <stdbool.h>
#include <String/rf_str_decl.h>
#include <ast/operators_decls.h> // for binary operations enum
#include <Data_Structures/intrusive_list.h>

struct rir_type;

// NOTE: preserve order, some functions depend on it
// order should be same as rir elementary types
enum elementary_type {
    ELEMENTARY_TYPE_INT_8 = 0,
    ELEMENTARY_TYPE_UINT_8,
    ELEMENTARY_TYPE_INT_16,
    ELEMENTARY_TYPE_UINT_16,
    ELEMENTARY_TYPE_INT_32,
    ELEMENTARY_TYPE_UINT_32,
    ELEMENTARY_TYPE_INT_64,
    ELEMENTARY_TYPE_UINT_64,
    ELEMENTARY_TYPE_INT,
    ELEMENTARY_TYPE_UINT,
    ELEMENTARY_TYPE_FLOAT_32,
    ELEMENTARY_TYPE_FLOAT_64,
    ELEMENTARY_TYPE_STRING,
    ELEMENTARY_TYPE_BOOL,
    ELEMENTARY_TYPE_NIL,

    ELEMENTARY_TYPE_TYPES_COUNT /* keep as last */
};

enum typeop_type {
    TYPEOP_INVALID,
    TYPEOP_SUM,
    TYPEOP_PRODUCT,
    TYPEOP_IMPLICATION
};

enum type_category {
    TYPE_CATEGORY_OPERATOR = 0,         /* a type combination of other types */
    TYPE_CATEGORY_LEAF,                 /* almost always part of another type */
    TYPE_CATEGORY_ELEMENTARY,           /* an elementary/builtin type */
    TYPE_CATEGORY_DEFINED,              /* a user defined type */
    TYPE_CATEGORY_WILDCARD,             /* the type of '_' */
    TYPE_CATEGORY_GENERIC,              /* a generic type as declared by the user */
};

struct type_elementary {
    //! What kind of elementary type this is
    enum elementary_type etype;
    //! If this is a type of a constant literal
    bool is_constant;
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

struct type_defined {
    const struct RFstring *name;
    struct type *type;
};

struct type {
    enum type_category category;
    //! The RIR version of the type
    const struct rir_type *rir_type;
    // TODO: deprecated member. Remove. No longer used.
    // list handler, to be added to either the types or the composite types list
    struct RFilist_node lh;
    union {
        struct type_defined defined;
        struct type_operator operator;
        struct type_leaf leaf;
        struct type_elementary elementary;
    };
};

#endif
