#ifndef LFR_IR_LTYPE_H
#define LFR_IR_LTYPE_H

#include <stdbool.h>
#include <Definitions/inline.h>
#include <types/type_decls.h>

struct rir_ctx;
struct rir_value;

enum rir_ltype_category {
    RIR_LTYPE_ELEMENTARY,
    RIR_LTYPE_COMPOSITE,
};

//! Represents a leaf type in the IR. Essentially a much simpler form of type
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

i_INLINE_DECL bool rir_ltype_is_specific_elementary(const struct rir_ltype *t,
                                                    enum elementary_type etype)
{
    return t->category == RIR_LTYPE_ELEMENTARY && t->etype == etype;
}

i_INLINE_DECL bool rir_ltype_is_composite(const struct rir_ltype *t)
{
    return t->category == RIR_LTYPE_COMPOSITE;
}
bool rir_ltype_is_union(const struct rir_ltype *t);

void rir_ltype_elem_init(struct rir_ltype *t, enum elementary_type etype);
struct rir_ltype *rir_ltype_elem_create(enum elementary_type etype, bool is_pointer);
struct rir_ltype *rir_ltype_elem_create_from_string(const struct RFstring *name, bool is_pointer);
struct rir_ltype *rir_ltype_comp_create(const struct rir_typedef *def, bool is_pointer);
struct rir_ltype *rir_ltype_create_from_type(const struct type *t, struct rir_ctx *ctx);
void rir_ltype_comp_init(struct rir_ltype *t, const struct rir_typedef *def, bool is_pointer);

struct rir_ltype *rir_ltype_copy_from_other(const struct rir_ltype *other);
struct rir_ltype *rir_ltype_create_from_other(const struct rir_ltype *other, bool is_pointer);

/**
 * @return true if two types are equal. Does not take pointers into account
 */
bool rir_ltype_equal(const struct rir_ltype *a, const struct rir_ltype *b);
/**
 * @return true if two types are equal and are also both pointers or not
 */
bool rir_ltype_identical(const struct rir_ltype *a, const struct rir_ltype *b);
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

const struct rir_ltype *rir_ltype_comp_member_type(const struct rir_ltype *t, uint32_t idx);
int rir_ltype_union_matched_type_from_fncall(const struct rir_ltype *t, const struct ast_node *n, struct rir_ctx *ctx);

void rir_ltype_destroy(struct rir_ltype *t);
#endif
