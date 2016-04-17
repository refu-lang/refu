#ifndef LFR_IR_TYPE_H
#define LFR_IR_TYPE_H

#include <stdbool.h>
#include <rflib/defs/inline.h>
#include <rflib/utils/sanity.h>
#include <types/type_decls.h>

struct rir_ctx;
struct rir;
struct rir_value;

enum rir_type_category {
    RIR_TYPE_ELEMENTARY,
    RIR_TYPE_COMPOSITE,
    RIR_TYPE_ARRAY
};

struct rir_type;

struct rir_type_array {
    const struct rir_type *type;
    // Size of the array if it's static and -1 if it's expandable
    int64_t size;
};

/**
 * Represents a leaf type in the IR. Essentially a much simpler form of type
 *
 * All rir types are owned by their respective memory and memory management is
 * left to the pool.
 */
struct rir_type {
    enum rir_type_category category;
    bool is_pointer;
    union {
        //! Elementary type
        enum elementary_type etype;
        //! Composite type
        const struct rir_typedef *tdef;
        //! Array type
        struct rir_type_array array;
    };
};

i_INLINE_DECL bool rir_type_is_elementary(const struct rir_type *t)
{
    return t->category == RIR_TYPE_ELEMENTARY;
}

i_INLINE_DECL bool rir_type_is_specific_elementary(const struct rir_type *t,
                                                   enum elementary_type etype)
{
    return t->category == RIR_TYPE_ELEMENTARY && t->etype == etype;
}

i_INLINE_DECL bool rir_type_is_composite(const struct rir_type *t)
{
    return t->category == RIR_TYPE_COMPOSITE;
}

i_INLINE_DECL int64_t rir_type_array_size(const struct rir_type *t)
{
    RF_ASSERT(t->category == RIR_TYPE_ARRAY, "Expected rir array type");
    return t->array.size;
}

bool rir_type_is_union(const struct rir_type *t);

void rir_type_elem_init(struct rir_type *t, enum elementary_type etype, bool is_pointer);
struct rir_type *rir_type_elem_get_or_create(
    struct rir *r,
    enum elementary_type etype,
    bool is_pointer
);
struct rir_type *rir_type_elem_get_from_string(
    struct rir *r,
    const struct RFstring *name,
    bool is_pointer
);

void rir_type_comp_init(struct rir_type *t, const struct rir_typedef *def, bool is_pointer);
struct rir_type *rir_type_comp_get_or_create(
    const struct rir_typedef *def,
    struct rir *r,
    bool is_pointer
);

void rir_type_arr_init(
    struct rir_type *t,
    const struct rir_type *pointing_type,
    int64_t size,
    bool is_pointer
);
struct rir_type *rir_type_arr_get_or_create(
    struct rir *r,
    const struct rir_type *pointing_type,
    int64_t size,
    bool is_pointer
);

struct rir_type *rir_type_create_from_type(const struct type *t, struct rir_ctx *ctx);
void rir_type_destroy(struct rir_type *t, struct rir *r);


/**
 * Retrieve a type based on @a other and the given arguments or create it if
 * it does not already exist.
 *
 * @param other          The type on which the returned type should be based
 * @param r              The rir module on which the type is based
 * @param is_pointer     Whether the returned type should be a pointer or not
 * @return               The requested type.
 */
struct rir_type *rir_type_get_or_create_from_other(
    const struct rir_type *other,
    struct rir *r,
    bool is_pointer
);

/**
 * @return true if two types are equal. Does not take pointers into account
 */
bool rir_type_equal(const struct rir_type *a, const struct rir_type *b);
/**
 * @return true if two types are equal and are also both pointers or not
 */
bool rir_type_identical(const struct rir_type *a, const struct rir_type *b);
size_t rir_type_bytesize(const struct rir_type *a);

/**
 * Create a temporary string representation of the type
 *
 * @note This call needs to be enclosed in RFS_PUSH/RFS_POP
 *
 * @param t         The type whose representation to get
 * @return          A string representing @a t
 */
const struct RFstring *rir_type_string(const struct rir_type *t);
/**
 * Returns a string representation of the category of the type
 */
const struct RFstring *rir_type_category_str(const struct rir_type *t);

const struct rir_type *rir_type_comp_member_type(const struct rir_type *t, uint32_t idx);
int rir_type_union_matched_type_from_fncall(
    const struct rir_type *t,
    const struct ast_node *n,
    struct rir_ctx *ctx
);

#endif
