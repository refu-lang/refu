#ifndef LFR_TYPES_DECL_H
#define LFR_TYPES_DECL_H

#include <stdbool.h>

#include <rfbase/string/decl.h>
#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/datastructs/darray.h>

#include <ast/operators_decls.h> // for binary operations enum

struct rir_type;
struct type;

//! An array of types
struct arr_types {darray(struct type*);};

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
    TYPE_CATEGORY_ELEMENTARY,           /* an elementary/builtin type */
    TYPE_CATEGORY_DEFINED,              /* a user defined type */
    TYPE_CATEGORY_WILDCARD,             /* the type of '_' */
    TYPE_CATEGORY_GENERIC,              /* a generic type as declared by the user */
    TYPE_CATEGORY_MODULE,               /* type of a module */
    TYPE_CATEGORY_ARRAY,                /* type of an array */
    TYPE_CATEGORY_TYPECLASS,            /* type of a typeclass */
    TYPE_CATEGORY_TYPEINSTANCE,         /* type of a typeinstance */
};

struct type_elementary {
    //! What kind of elementary type this is
    enum elementary_type etype;
};

struct type_operator {
    enum typeop_type type;
    //! Types that consitute this type
    struct arr_types operands;
};

struct type_defined {
    const struct RFstring *name;
    struct type *type;
};

struct type_simple {
    const struct RFstring *name;
};

struct arr_int64 { darray(int64_t);};

struct type_array {
    struct arr_int64 dimensions;
    const struct type *member_type;
};

struct type {
    enum type_category category;
    bool is_constant;
    union {
        struct type_defined defined;
        struct type_operator operator;
        struct type_elementary elementary;
        struct type_simple simple;
        struct type_array array;
    };
};
#endif
