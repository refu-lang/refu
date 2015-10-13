#ifndef LFR_IR_TYPE_DECLS_H
#define LFR_IR_TYPE_DECLS_H

#include <Data_Structures/darray.h>
#include <Data_Structures/intrusive_list.h>

//! Keep synced with @ref elementary_types at type_decls.h
enum rir_type_category {
    ELEMENTARY_RIR_TYPE_INT_8 = 0,
    ELEMENTARY_RIR_TYPE_UINT_8,
    ELEMENTARY_RIR_TYPE_INT_16,
    ELEMENTARY_RIR_TYPE_UINT_16,
    ELEMENTARY_RIR_TYPE_INT_32,
    ELEMENTARY_RIR_TYPE_UINT_32,
    ELEMENTARY_RIR_TYPE_INT_64,
    ELEMENTARY_RIR_TYPE_UINT_64,
    ELEMENTARY_RIR_TYPE_INT,
    ELEMENTARY_RIR_TYPE_UINT,
    ELEMENTARY_RIR_TYPE_FLOAT_32,
    ELEMENTARY_RIR_TYPE_FLOAT_64,
    ELEMENTARY_RIR_TYPE_STRING,
    ELEMENTARY_RIR_TYPE_BOOL,
    RIR_TYPE_WILDCARD,        // treat wildcard as an elementary type
    ELEMENTARY_RIR_TYPE_NIL,

    COMPOSITE_PRODUCT_RIR_TYPE,
    COMPOSITE_SUM_RIR_TYPE,
    COMPOSITE_IMPLICATION_RIR_TYPE,
    COMPOSITE_RIR_DEFINED,

    RIR_TYPE_CATEGORY_COUNT
};

/**
 * Representation of a type for the Refu IR
 *
 * It is much like @see struct type but with some constraints.
 * It is represented by an array of subtypes and not by a tree. A single
 * rir_type can only contain subtypes that are connected by the same type
 * operation.
 */
struct rir_type {
    enum rir_type_category category;
    //! Array of types that may constitute this type. e.g: i64, u32, string
    struct {darray(struct rir_type*);} subtypes;
    //! Name of the variable the type describes. TODO: Maybe move this somwhere
    //! else. Separate the notion of type from parameter?
    const struct RFstring *name;
    //! A pointer to the normal type from which this rir type was created.
    const struct type *type;
    //! Control to input into the rir types list
    struct RFilist_node ln;
    //! Denotes if the type is indexed in the rir types list (or some other list)
    //! that takes care of destruction. If so then it's not destroyed when it's found
    //! as a child of another type. TODO: This is kind of ugly .. if possible fix
    bool indexed;
};
#endif
