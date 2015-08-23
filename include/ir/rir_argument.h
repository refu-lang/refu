#ifndef LFR_IR_ARGUMENT_H
#define LFR_IR_ARGUMENT_H

#include <stdbool.h>
#include <types/type_decls.h>
#include <Data_Structures/darray.h>

struct rir;
struct rir_type;

enum rir_ltype_category {
    RIR_LTYPE_ELEMENTARY,
    RIR_LTYPE_COMPOSITE,
};

//! Represents a leaf type in the IR. Essentially a much simpler for of rir_type.
struct rir_ltype {
    enum rir_ltype_category category;
    union {
        //! Elementary type
        enum elementary_type etype;
        //! Composite type
        const struct rir_typedef *tdef;
    };
};

struct rir_ltype *rir_ltype_elem_create(enum elementary_type etype);
struct rir_ltype *rir_ltype_comp_create(struct rir_typedef *def);

size_t rir_ltype_bytesize(const struct rir_ltype *a);

void rir_ltype_destroy(struct rir_ltype *t);

//! Represents a leaf argument on the IR. e.g. a:i32
struct rir_argument {
    //! The type of the leaf
    struct rir_ltype type;
    //! An optional name for the argument.
    const struct RFstring *name;
};

struct rir_argument *rir_argument_create(const struct rir_type *type);
struct rir_argument *rir_argument_create_from_typedef(const struct rir_typedef *d);
void rir_argument_destroy(struct rir_argument *a);
bool rir_argument_tostring(struct rir *r, const struct rir_argument *arg);

/* -- Functions dealing with argument arrays -- */

struct args_arr {darray(const struct rir_argument*);};
bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr);
bool rir_argsarr_tostring(struct rir *r, const struct args_arr *arr);
#endif
