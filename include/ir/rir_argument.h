#ifndef LFR_IR_ARGUMENT_H
#define LFR_IR_ARGUMENT_H

#include <stdbool.h>
#include <types/type_decls.h>
#include <Data_Structures/darray.h>

struct rirtostr_ctx;
struct rir_type;
struct rir;

enum rir_ltype_category {
    RIR_LTYPE_ELEMENTARY,
    RIR_LTYPE_COMPOSITE,
};

//! Represents a leaf type in the IR. Essentially a much simpler for of rir_type.
struct rir_ltype {
    enum rir_ltype_category category;
    bool is_pointer;
    union {
        //! Elementary type
        enum elementary_type etype;
        //! Composite type
        const struct rir_typedef *tdef;
    };
};

i_INLINE_DECL bool rir_ltype_is_elementary(const struct rir_ltype *t)
{
    return t->category == RIR_LTYPE_ELEMENTARY;
}

i_INLINE_DECL bool rir_ltype_is_composite(const struct rir_ltype *t)
{
    return t->category == RIR_LTYPE_COMPOSITE;
}
bool rir_ltype_is_union(const struct rir_ltype *t);

struct rir_ltype *rir_ltype_elem_create(enum elementary_type etype, bool is_pointer);
struct rir_ltype *rir_ltype_elem_create_from_string(const struct RFstring *name, bool is_pointer);
struct rir_ltype *rir_ltype_comp_create(const struct rir_typedef *def, bool is_pointer);

struct rir_ltype *rir_ltype_copy_from_other(const struct rir_ltype *other);
struct rir_ltype *rir_ltype_create_from_other(const struct rir_ltype *other, bool is_pointer);

size_t rir_ltype_bytesize(const struct rir_ltype *a);

/**
 * Create a temporary string representation of the type
 *
 * @note This call needs to be enclosed in RFS_PUSH/RFS_POP
 *
 * @param t         The type whose representation to get
 * @return          A string representing @a t
 */
const struct RFstring *rir_ltype_string(const struct rir_ltype *t);

const struct rir_ltype *rir_ltype_comp_member_type(const struct rir_ltype *t, unsigned int i);
int rir_ltype_union_matched_type_from_fncall(const struct rir_ltype *t, const struct ast_node *n, const struct rir *r);

void rir_ltype_destroy(struct rir_ltype *t);



//! Represents a leaf argument on the IR. e.g. a:i32
struct rir_argument {
    //! The type of the leaf
    struct rir_ltype type;
    //! An optional name for the argument.
    const struct RFstring *name;
};

struct rir_argument *rir_argument_create(const struct rir_type *type, const struct rir *r);
struct rir_argument *rir_argument_create_from_typedef(const struct rir_typedef *d);
void rir_argument_destroy(struct rir_argument *a);
bool rir_argument_tostring(struct rirtostr_ctx *ctx, const struct rir_argument *arg);

/* -- Functions dealing with argument arrays -- */

struct args_arr {darray(struct rir_argument*);};
bool rir_type_to_arg_array(const struct rir_type *type, struct args_arr *arr, const struct rir *r);
bool rir_argsarr_tostring(struct rirtostr_ctx *ctx, const struct args_arr *arr);
bool rir_argsarr_equal(const struct args_arr *arr1, const struct args_arr *arr2);
void rir_argsarr_deinit(struct args_arr *arr);
#endif
