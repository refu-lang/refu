#include <ir/rir_type.h>

#include <String/rf_str_core.h>

#include <ast/type.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

static bool rir_type_init_iteration(struct rir_type *type, const struct type *input,
                                    const struct RFstring *name,
                                    enum rir_type_category *previous_type_op,
                                    struct rir_type **newly_created_type)
{
    enum rir_type_category op_category;
    struct rir_type *new_type;
    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        if (type_elementary(input) != ELEMENTARY_TYPE_NIL) {
            new_type = rir_type_create(input, name, NULL);
            darray_append(type->subtypes, new_type);
        }
        break;
    case TYPE_CATEGORY_DEFINED:
        type->category = COMPOSITE_RIR_DEFINED;
        type->name = input->defined.name;
        new_type = rir_type_create(input->defined.type, NULL, NULL);
        if (!new_type) {
            return false;
        }
        darray_append(type->subtypes, new_type);
        break;
    case TYPE_CATEGORY_OPERATOR:
        op_category = rir_type_op_from_type(input);

        if (*previous_type_op == RIR_TYPE_CATEGORY_COUNT) {
            *previous_type_op = op_category;
            new_type = type;
            new_type->category = op_category;
        } else if (*previous_type_op != op_category) {
            new_type = rir_type_create(input, NULL, NULL);
            new_type->category = op_category;
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        } else {
            new_type = type;
            new_type->category = op_category;
        }

        if (!rir_type_init_iteration(new_type, input->operator.left, NULL, previous_type_op, newly_created_type)) {
            return false;
        }

        if (*previous_type_op != op_category) {
            new_type = rir_type_create(input->operator.right, NULL, NULL);
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        }

        if (!rir_type_init_iteration(new_type, input->operator.right, NULL, previous_type_op, newly_created_type)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        if (!rir_type_init_iteration(type, input->leaf.type, input->leaf.id, previous_type_op, newly_created_type)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_GENERIC:
        RF_ASSERT(false, "Generic types not supported in the IR yet");
        break;
    case TYPE_CATEGORY_WILDCARD:
        RF_ASSERT(false, "Wildmard type should not appear in the IR");
                  break;
    }
    return true;
}

bool rir_type_init_before_iteration(struct rir_type *type,
                                    const struct type *input,
                                    const struct RFstring *name)
{
    darray_init(type->subtypes);
    type->name = name;
    type->indexed = false;

    // for elementary types there is nothing to inialize
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        type->category = (enum rir_type_category) type_elementary(input);
        return false;
    } else if (input->category == TYPE_CATEGORY_LEAF &&
        input->leaf.type->category == TYPE_CATEGORY_ELEMENTARY) {
        // for leafs, that are elementary types, just create a named elementary rir type
        type->category = (enum rir_type_category) type_elementary(input->leaf.type);
        type->name = input->leaf.id;
        return false;
    }
    return true;
}

bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name,
                   struct rir_type **newly_created_type)
{

    if (!rir_type_init_before_iteration(type, input, name)) {
        return true; // no need to iterate children of input type
    }
    // iterate the subtypes
    enum rir_type_category previous_type_op = RIR_TYPE_CATEGORY_COUNT;
    if (!rir_type_init_iteration(type, input, name, &previous_type_op, newly_created_type)) {
        return false;
    }

    return true;
}

struct rir_type *rir_type_alloc()
{
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    return ret;
}

struct rir_type *rir_type_create(const struct type *input,
                                 const struct RFstring *name,
                                 struct rir_type **newly_created_type)
{
    struct rir_type *ret = rir_type_alloc();
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input, name, newly_created_type)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }

    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    free(t);
}
void rir_type_destroy(struct rir_type *t)
{
    rir_type_deinit(t);
    rir_type_dealloc(t);
}
void rir_type_deinit(struct rir_type *t)
{
    struct rir_type **subtype;
    darray_foreach(subtype, t->subtypes) {
        if (!(*subtype)->indexed) {
            rir_type_destroy(*subtype);
        }
    }

    darray_free(t->subtypes);
}

enum rir_type_category rir_type_op_from_rir_type(const struct rir_type *t)
{
    if (t->category >= COMPOSITE_PRODUCT_RIR_TYPE && t->category <= COMPOSITE_IMPLICATION_RIR_TYPE) {
        return t->category;
    }
    return RIR_TYPE_CATEGORY_COUNT;
}

enum rir_type_category rir_type_op_from_type(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_OPERATOR, "Invalid type");
    switch (t->operator.type) {
    case TYPEOP_PRODUCT:
        return COMPOSITE_PRODUCT_RIR_TYPE;
    case TYPEOP_SUM:
        return COMPOSITE_SUM_RIR_TYPE;
    case TYPEOP_IMPLICATION:
        return COMPOSITE_IMPLICATION_RIR_TYPE;
    default:
        RF_ASSERT(false, "Illegal type operation encountered");
        return -1;
    }
}

bool rir_type_equals(struct rir_type *a, struct rir_type *b)
{
    struct rir_type **subtype_a = NULL;
    unsigned int i = 0;
    if (a->category != b->category) {
        return false;
    }

    if ((a->name == NULL &&  b->name != NULL) ||
        (a->name != NULL && b->name == NULL)) {
        return false;
    } else if (a->name != NULL && b->name != NULL &&
               !rf_string_equal(a->name, b->name)) {
        return false;
    }

    if (darray_size(a->subtypes) != darray_size(b->subtypes)) {
        return false;
    }

    darray_foreach(subtype_a, a->subtypes) {
        if (!rir_type_equals(*subtype_a, darray_item(b->subtypes, i))) {
            return false;
        }
        ++i;
    }

    return true;
}


bool rir_type_is_subtype_of_other(struct rir_type *t,
                                  struct rir_type *other)
{
    struct rir_type **subtype;
    darray_foreach(subtype, other->subtypes) {
        if (*subtype == t) {
            return true;
        }
        if (darray_size((*subtype)->subtypes) != 0) {
            if (rir_type_is_subtype_of_other(t, *subtype)) {
                return true;
            }
        }
    }
    return false;
}

struct rir_type_cmp_ctx {
    //! Current type operation while iterating the rir type to type comparison
    enum rir_type_category current_rir_op;
    //! A stack of the currently visited rir types
    struct {darray(struct rir_type*);} rir_types;
    //! A stack of the indices of the currently visited rir types
    darray(int) indices;
};

static inline void rir_type_cmp_ctx_push_type(struct rir_type_cmp_ctx *ctx, struct rir_type *rir_type)
{
    darray_push(ctx->rir_types, rir_type);
    darray_push(ctx->indices, -1);
}

static inline struct rir_type *rir_type_cmp_ctx_current_type(struct rir_type_cmp_ctx *ctx)
{
    if (darray_top(ctx->indices) == -1) {
        return darray_top(ctx->rir_types);
    }
    /* RF_ASSERT(darray_top(ctx->indices) < (int)darray_size(darray_top(ctx->rir_types)->subtypes), */
    /*           "Attempt to access subtype out of bounds. Should not happen at this point."); */
    if (darray_top(ctx->indices) >= (int)darray_size(darray_top(ctx->rir_types)->subtypes)) {
        return NULL;
    }
    return darray_item(darray_top(ctx->rir_types)->subtypes, darray_top(ctx->indices));
}

static inline void rir_type_cmp_ctx_idx_plus1(struct rir_type_cmp_ctx *ctx)
{
    darray_top(ctx->indices) = darray_top(ctx->indices) + 1;
}

static inline void rir_type_cmp_ctx_go_up(struct rir_type_cmp_ctx *ctx, struct type *t)
{
    enum rir_type_category op = rir_type_op_from_type(t);
    if (op != ctx->current_rir_op) {
        (void)darray_pop(ctx->indices);
        (void)darray_pop(ctx->rir_types);

        // also go to next index of the previous type
        ctx->current_rir_op = op;
        rir_type_cmp_ctx_idx_plus1(ctx);
    }
}

void rir_type_cmp_ctx_init(struct rir_type_cmp_ctx *ctx, struct rir_type *rir_type)
{
    darray_init(ctx->rir_types);
    darray_init(ctx->indices);
    rir_type_cmp_ctx_push_type(ctx, rir_type);
    ctx->current_rir_op = RIR_TYPE_CATEGORY_COUNT;
}

void rir_type_cmp_ctx_deinit(struct rir_type_cmp_ctx *ctx)
{
    darray_free(ctx->rir_types);
    darray_free(ctx->indices);
}

bool rir_type_cmp_pre_cb(struct type *t, struct rir_type_cmp_ctx *ctx)
{
    struct rir_type *current_rir = rir_type_cmp_ctx_current_type(ctx);
    if (!current_rir) {
        return false;
    }
    if (t->category == TYPE_CATEGORY_OPERATOR) {
        enum rir_type_category n_type_op = rir_type_op_from_type(t);
        if (ctx->current_rir_op == RIR_TYPE_CATEGORY_COUNT) {
            ctx->current_rir_op = n_type_op;
            if (rir_type_op_from_rir_type(current_rir) == RIR_TYPE_CATEGORY_COUNT) {
                return false;
            }
            // also go to the first index of the type
            rir_type_cmp_ctx_idx_plus1(ctx);
        } else {
            // check if we need to go deeper into the rir type
            if (n_type_op != ctx->current_rir_op) {
                rir_type_cmp_ctx_push_type(ctx, current_rir);

                // also go to the first index of the new type
                ctx->current_rir_op = n_type_op;
                rir_type_cmp_ctx_idx_plus1(ctx);
            }
        }

    } else if (t->category == TYPE_CATEGORY_DEFINED) {
        if (current_rir->category != COMPOSITE_RIR_DEFINED) {
            return false;
        }

        if (!rf_string_equal(current_rir->name, t->defined.name)) {
            return false;
        }

        rir_type_cmp_ctx_push_type(ctx, current_rir->subtypes.item[0]);
    }
    // for other type categories do nothing
    return true;
}

bool rir_type_cmp_post_cb(struct type *t, struct rir_type_cmp_ctx *ctx)
{
    struct rir_type *curr_rir;

    switch (t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        curr_rir = rir_type_cmp_ctx_current_type(ctx);
        if (!curr_rir) {
            return false;
        }
        if (!rir_type_is_elementary(curr_rir)) {
            return false;
        }
        return (enum elementary_type)curr_rir->category == type_elementary(t);

    case TYPE_CATEGORY_DEFINED:
        break;

    case TYPE_CATEGORY_OPERATOR:
        rir_type_cmp_ctx_go_up(ctx, t);
        break;

    case TYPE_CATEGORY_LEAF:
        curr_rir = rir_type_cmp_ctx_current_type(ctx);
        if (!curr_rir) {
            return false;
        }
        if (!curr_rir->name || !rf_string_equal(curr_rir->name, t->leaf.id)) {
            return false;
        }
        if (!rir_type_equals_type(curr_rir, t->leaf.type, NULL)) {
            return false;
        }
        // also index + 1
        rir_type_cmp_ctx_idx_plus1(ctx);
        break;
    default:
        RF_ASSERT(false, "Illegal type category for comparison");
        return false;
    }
    return true;
}

bool rir_type_equals_type(struct rir_type *r_type, struct type *n_type, const struct RFstring *name)
{
    struct rir_type_cmp_ctx ctx;
    bool ret;

    if (name && (!r_type->name || !rf_string_equal(name, r_type->name))) {
        return false;
    }

    rir_type_cmp_ctx_init(&ctx, r_type);
    ret = type_traverse(n_type, (type_iterate_cb)(rir_type_cmp_pre_cb),
                        (type_iterate_cb)(rir_type_cmp_post_cb), &ctx);
    rir_type_cmp_ctx_deinit(&ctx);
    return ret;
}

const struct RFstring *rir_type_get_nth_name(struct rir_type *t, unsigned n)
{
    // if it's only 1 simple type and we want the first subtype's name it's this one
    if (n == 0 && darray_size(t->subtypes) == 0) {
        return t->name;
    }
    if (darray_size(t->subtypes) <= n) {
        return NULL;
    }
    return ((struct rir_type*)darray_item(t->subtypes, n))->name;
}
i_INLINE_INS const struct RFstring *rir_type_get_nth_name_or_die(struct rir_type *t, unsigned n);

const struct rir_type *rir_type_get_nth_type(struct rir_type *t, unsigned n)
{
    // if it's only 1 simple type and we want the first subtype's type it's this one
    if (n == 0 && darray_size(t->subtypes) == 0) {
        return t;
    }
    if (darray_size(t->subtypes) <= n) {
        RF_ERROR("Requested rir_type type of subtype out of bounds");
        return NULL;
    }
    return darray_item(t->subtypes, n);
}
i_INLINE_INS const struct rir_type *rir_type_get_nth_type_or_die(struct rir_type *t, unsigned n);

static inline const struct RFstring *rir_type_op_to_str(const struct rir_type *t)
{
    switch(t->category) {
    case COMPOSITE_PRODUCT_RIR_TYPE:
        return type_op_str(TYPEOP_PRODUCT);
    case COMPOSITE_SUM_RIR_TYPE:
        return type_op_str(TYPEOP_SUM);
    case COMPOSITE_IMPLICATION_RIR_TYPE:
        return type_op_str(TYPEOP_IMPLICATION);
    default:
        RF_ASSERT(false, "Invalid rir_type at rir_type_op_to_str()");
        break;
    }

    return NULL;
}

static const struct RFstring rir_op_names[] = {
    [COMPOSITE_PRODUCT_RIR_TYPE] = RF_STRING_STATIC_INIT("product_type"),
    [COMPOSITE_SUM_RIR_TYPE] = RF_STRING_STATIC_INIT("sum_type"),
    [COMPOSITE_IMPLICATION_RIR_TYPE] = RF_STRING_STATIC_INIT("implication_type"),
};

struct RFstring *rir_type_str(const struct rir_type *t)
{
    struct RFstring *s;
    struct rir_type **subtype;
    unsigned int count = 1;
    if (rir_type_is_elementary(t)) {
        return t->name
            ? RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                  RF_STR_PF_ARG(t->name),
                  RF_STR_PF_ARG(type_elementary_get_str((enum elementary_type)t->category)))
            : RFS(RF_STR_PF_FMT,
                RF_STR_PF_ARG(type_elementary_get_str((enum elementary_type)t->category)));
    }

    switch(t->category) {
    case COMPOSITE_PRODUCT_RIR_TYPE:
    case COMPOSITE_SUM_RIR_TYPE:
    case COMPOSITE_IMPLICATION_RIR_TYPE:
        s = t->name
            ? RFS(RF_STR_PF_FMT ":" RF_STR_PF_FMT"( ",
                  RF_STR_PF_ARG(t->name),
                  RF_STR_PF_ARG(&rir_op_names[t->category]))
            : RFS(RF_STR_PF_FMT"( ",
                  RF_STR_PF_ARG(&rir_op_names[t->category]));
        if (!s) {
            return NULL;
        }

        darray_foreach(subtype, t->subtypes) {
            struct RFstring *subtypes = rir_type_str(*subtype);
            if (!subtypes) {
                return NULL;
            }
            s = (darray_size(t->subtypes) == count)
                ? RFS(RF_STR_PF_FMT RF_STR_PF_FMT,
                      RF_STR_PF_ARG(s),
                      RF_STR_PF_ARG(subtypes))
            : RFS(RF_STR_PF_FMT RF_STR_PF_FMT RF_STR_PF_FMT,
                  RF_STR_PF_ARG(s),
                  RF_STR_PF_ARG(subtypes),
                  RF_STR_PF_ARG(rir_type_op_to_str(t)));
            if (!s) {
                return NULL;
            }
            ++ count;
        }
        return RFS(RF_STR_PF_FMT " )", RF_STR_PF_ARG(s));

    case COMPOSITE_RIR_DEFINED:
        s = rir_type_str(darray_item(t->subtypes, 0));
        return s
            ? RFS("type "RF_STR_PF_FMT"{ " RF_STR_PF_FMT " }",
                  RF_STR_PF_ARG(t->name),
                  RF_STR_PF_ARG(s))
            : NULL;
    default:
        RF_ASSERT(false, "Invalid rir_type at rir_type_str()");
        break;
    }
    return NULL;
}
i_INLINE_INS struct RFstring *rir_type_str_or_die(const struct rir_type *t);

i_INLINE_INS bool rir_type_is_elementary(const struct rir_type *t);
