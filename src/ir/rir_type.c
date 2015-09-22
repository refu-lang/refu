#include <ir/rir_type.h>

#include <String/rf_str_core.h>
#include <Utils/bits.h>

#include <ast/type.h>
#include <analyzer/type_set.h>

#include <ir/rir_types_list.h>

#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

struct rir_type *rir_type_find_or_create(struct type *check_type,
                                         const struct RFstring *name,
                                         struct RFilist_head *list,
                                         bool *found)
{
    struct rir_type *iter_rir_type;
    iter_rir_type = rir_types_list_get_type(list, check_type, name);
    *found = true;
    if (!iter_rir_type) {
        *found = false;
        iter_rir_type = rir_type_create(check_type, name, list);
    }
    return iter_rir_type;
}

static inline bool rir_type_find_or_create_append(struct rir_type *parent,
                                                  struct type *check_type,
                                                  const struct RFstring *name,
                                                  struct RFilist_head *list)
{
    bool equivalent_in_list;
    struct rir_type *type = rir_type_find_or_create(check_type, name, list, &equivalent_in_list);
    if (!type) {
        RF_ERROR("Could not find or create a rir type");
        return false;
    }
    if (!rir_types_list_has(list, type) && !equivalent_in_list) {
        rf_ilist_add_tail(list, &type->ln);
        type->indexed = true;
    }
    darray_append(parent->subtypes, type);
    return true;
}

static bool rir_type_init_iteration(struct rir_type *type,
                                    struct type *input,
                                    const struct RFstring *name,
                                    enum rir_type_category *previous_type_op,
                                    struct RFilist_head *list)
{
    enum rir_type_category op_category;
    struct rir_type *new_type;
    bool equivalent_in_list;
    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        if (type_elementary(input) == ELEMENTARY_TYPE_NIL) {
            break;
        }
        // else wildcards and elementary are treated the same
    case TYPE_CATEGORY_WILDCARD:
        new_type = rir_type_create(input, name, list);
        darray_append(type->subtypes, new_type);
        if (!rir_types_list_has(list, new_type)) {
            rf_ilist_add_tail(list, &new_type->ln);
            new_type->indexed = true;
        }
        break;
    case TYPE_CATEGORY_DEFINED:
        type->category = COMPOSITE_RIR_DEFINED;
        type->name = input->defined.name;
        // here since rir_type_create() gets the type of the defined, we have to
        // manually set tne rir type of input
        input->rir_type = type;
        return rir_type_find_or_create_append(type, input->defined.type, NULL, list);
    case TYPE_CATEGORY_OPERATOR:
        op_category = rir_type_op_from_type(input);

        if (*previous_type_op == RIR_TYPE_CATEGORY_COUNT) {
            *previous_type_op = op_category;
            new_type = type;
            new_type->category = op_category;
        } else if (*previous_type_op != op_category) {
            new_type = rir_type_find_or_create(input, NULL, list, &equivalent_in_list);
            new_type->category = op_category;
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        } else {
            new_type = type;
            new_type->category = op_category;
        }

        if (!rir_type_init_iteration(new_type, input->operator.left, NULL, previous_type_op, list)) {
            return false;
        }

        if (*previous_type_op != op_category) {
            new_type = rir_type_find_or_create(input->operator.right, NULL, list, &equivalent_in_list);
            *previous_type_op = op_category;
            darray_append(type->subtypes, new_type);
            return true;
        }

        if (!rir_type_init_iteration(new_type, input->operator.right, NULL, previous_type_op, list)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        return rir_type_find_or_create_append(type, input->leaf.type, input->leaf.id, list);
    case TYPE_CATEGORY_MODULE:
        RF_CRITICAL_FAIL("Module types not supported in the IR");
        break;
    case TYPE_CATEGORY_GENERIC:
        RF_CRITICAL_FAIL("Generic types not supported in the IR yet");
        break;
    }
    return true;
}

bool rir_type_init_before_iteration(struct rir_type *type,
                                    struct type *input,
                                    const struct RFstring *name)
{
    RF_STRUCT_ZERO(type);
    darray_init(type->subtypes);
    type->name = name;
    type->indexed = false;
    type->type = input;
    // keep the rir type version of the type
    input->rir_type = type;
    // for elementary types there is nothing to initialize
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        type->category = (enum rir_type_category) type_elementary(input);
        return false;
    } else if (input->category == TYPE_CATEGORY_WILDCARD) {
        type->category = RIR_TYPE_WILDCARD;
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

bool rir_type_init(struct rir_type *type,
                   struct type *input,
                   const struct RFstring *name,
                   struct RFilist_head* list)
{

    if (!rir_type_init_before_iteration(type, input, name)) {
        return true; // no need to iterate children of input type
    }
    // iterate the subtypes
    enum rir_type_category previous_type_op = RIR_TYPE_CATEGORY_COUNT;
    if (!rir_type_init_iteration(type, input, name, &previous_type_op, list)) {
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

struct rir_type *rir_type_create(struct type *input,
                                 const struct RFstring *name,
                                 struct RFilist_head* list)
{
    struct rir_type *ret = rir_type_alloc();
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input, name, list)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }
    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    free(t);
}

static void rir_type_deinit_check_list(struct rir_type *t, struct RFilist_head *list)
{
    struct rir_type **subtype;
    darray_foreach(subtype, t->subtypes) {
        if (!rf_ilist_has(list, &(*subtype)->ln)) {
            rir_type_destroy_check_list(*subtype, list);
        }
    }
    darray_free(t->subtypes);
}

void rir_type_destroy_check_list(struct rir_type *t, struct RFilist_head *list)
{
    if (rir_type_statically_alloced(t)) {
        return;
    }
    rir_type_deinit_check_list(t, list);
    rir_type_dealloc(t);
}
void rir_type_destroy(struct rir_type *t)
{
    if (rir_type_statically_alloced(t)) {
        return;
    }
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
        RF_CRITICAL_FAIL("Illegal type operation encountered");
        return -1;
    }
}


static inline bool rir_type_op_is_sumtype(const struct rir_type *t)
{
    return darray_size(t->subtypes) && t->category == COMPOSITE_SUM_RIR_TYPE;
}

 bool rir_type_defined_is_sumtype(const struct rir_type *t)
{
    RF_ASSERT(t->category == COMPOSITE_RIR_DEFINED, "Called with non defined type");
    RF_ASSERT(darray_size(t->subtypes) == 1,
              "A defined type should always have 1 direct subtype");
    
    return rir_type_is_sumtype(darray_item(t->subtypes, 0));
}

bool rir_type_is_sumtype(const struct rir_type *t)
{
    if (t->category == COMPOSITE_RIR_DEFINED) {
        return rir_type_defined_is_sumtype(t);
    }
    return rir_type_op_is_sumtype(t);
}


bool rir_type_equals(const struct rir_type *a,
                     const struct rir_type *b,
                     enum rir_typecmp_options options)
{
    struct rir_type **subtype_a = NULL;
    unsigned int i = 0;

    if (RF_BITFLAG_ON(options, RIR_TYPECMP_NAMES)) {
        if ((a->name == NULL &&  b->name != NULL) ||
            (a->name != NULL && b->name == NULL)) {
            return false;
        } else if (a->name != NULL && b->name != NULL &&
                   !rf_string_equal(a->name, b->name)) {
            return false;
        }
    }

    if (a->category != b->category) {
        if (RF_BITFLAG_ON(options, RIR_TYPECMP_CONVERTABLE)) {
            // TODO: this has to be more sophisticated just like in typecmp
            // but perhaps better approach is to avoid this totally here and
            // not having to rely on any type comparisons at this stage.
            // Since actual type checking occurred at the previous stage this will
            // work most of the times but it feels hacky as hell ...
            if ((a->category <= ELEMENTARY_RIR_TYPE_UINT && b->category <= ELEMENTARY_RIR_TYPE_UINT) ||
                (a->category >= ELEMENTARY_RIR_TYPE_FLOAT_32 && a->category <= ELEMENTARY_RIR_TYPE_FLOAT_64 &&
                 b->category >= ELEMENTARY_RIR_TYPE_FLOAT_32 && b->category <= ELEMENTARY_RIR_TYPE_FLOAT_64)) {
                // no need to check for children, elementary types have no children
                return true;
            }
        }
        return false;
    }

    if (darray_size(a->subtypes) != darray_size(b->subtypes)) {
        return false;
    }

    darray_foreach(subtype_a, a->subtypes) {
        if (!rir_type_equals(*subtype_a, darray_item(b->subtypes, i), options)) {
            return false;
        }
        ++i;
    }

    return true;
}

int rir_type_childof_type(const struct rir_type *t, const struct rir_type *maybe_parent)
{
    struct rir_type **subtype;
    int i = 0;
    darray_foreach(subtype, rir_type_contents(maybe_parent)->subtypes) {
        if (rir_type_equals(t, *subtype, RIR_TYPECMP_SIMPLE)) {
            return i;
        }
        ++i;
    }
    return -1;
}

// temporary macro to trace the logic of equality type to rir type equality comparison
/* #define RF_DEBUG_TRACE_RIR_TYPE_EQUALS */
#ifdef RF_DEBUG_TRACE_RIR_TYPE_EQUALS
#define DD(...) do { RFS_PUSH(); RF_CDEBUG(__VA_ARGS__); RFS_POP();}while(0)
#else
#define DD(...) 
#endif

struct rir_type_cmp_ctx {
    //! Current type operation while iterating the rir type to type comparison
    enum rir_type_category current_rir_op;
    //! A stack of the currently visited rir types
    struct {darray(const struct rir_type*);} rir_types;
    //! A stack of the indices of the currently visited rir types
    darray(int) indices;
};

static inline void rir_type_cmp_ctx_push_type(struct rir_type_cmp_ctx *ctx,
                                              const struct rir_type *rir_type)
{
    darray_push(ctx->rir_types, rir_type);
    darray_push(ctx->indices, -1);
}

static inline const struct rir_type *rir_type_cmp_ctx_current_type(struct rir_type_cmp_ctx *ctx)
{
    if (darray_top(ctx->indices) == -1) {
        return darray_top(ctx->rir_types);
    }
    if (darray_top(ctx->indices) >= (int)darray_size(darray_top(ctx->rir_types)->subtypes)) {
        return NULL;
    }
    return darray_item(darray_top(ctx->rir_types)->subtypes, darray_top(ctx->indices));
}

static inline void rir_type_cmp_ctx_idx_plus1(struct rir_type_cmp_ctx *ctx)
{
    darray_top(ctx->indices) = darray_top(ctx->indices) + 1;
    DD("type \""RF_STR_PF_FMT"\" at index [%lu] += 1 --> %u\n",
       RF_STR_PF_ARG(rir_type_str_or_die(darray_top(ctx->rir_types))),
       darray_size(ctx->indices) - 1, darray_top(ctx->indices));
}

static inline void rir_type_cmp_ctx_go_up(struct rir_type_cmp_ctx *ctx, struct type *t)
{
    enum rir_type_category op = t->category == TYPE_CATEGORY_DEFINED
        ? rir_type_op_from_type(t->defined.type)
        : rir_type_op_from_type(t);
    if (op != ctx->current_rir_op) {
#ifdef RF_DEBUG_TRACE_RIR_TYPE_EQUALS
        if (darray_size(ctx->rir_types) > 1) {
            DD("ctx type going up. Switching from \""RF_STR_PF_FMT"\" to \""RF_STR_PF_FMT"\".\n",
               RF_STR_PF_ARG(rir_type_str_or_die(darray_top(ctx->rir_types))),
               RF_STR_PF_ARG(rir_type_str_or_die(darray_item(ctx->rir_types, darray_size(ctx->rir_types) - 2))));
        } else {
            DD("ctx type going up. Switching out \""RF_STR_PF_FMT"\" which was the last type .\n",
               RF_STR_PF_ARG(rir_type_str_or_die(darray_top(ctx->rir_types))));
        }
#endif
        (void)darray_pop(ctx->indices);
        (void)darray_pop(ctx->rir_types);

        // also go to next index of the previous type
        ctx->current_rir_op = op;
        rir_type_cmp_ctx_idx_plus1(ctx);
    }
}

static void rir_type_cmp_ctx_init(struct rir_type_cmp_ctx *ctx, const struct rir_type *rir_type)
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

/**
 * The pre callback is called during the comparison traversal "going downwards"
 * in the type. It contains the logic that checks if a type is properly separated
 * into subtypes as expected for the given type argument. Modifies the comparison
 * context accordingly
 *
 * @param t        The part of the original type we are comparing at this moment
 * @param ctx      The comparison context holding all the necessary information
 * @return         true if comparison can continue, false if we got a comparison failure
 */
bool rir_type_cmp_pre_cb(struct type *t, struct rir_type_cmp_ctx *ctx)
{
    const struct rir_type *current_rir = rir_type_cmp_ctx_current_type(ctx);
    if (!current_rir) {
        DD("!current_rir -> return false\n");
        return false;
    }
    if (t->category == TYPE_CATEGORY_OPERATOR) {
        enum rir_type_category n_type_op = rir_type_op_from_type(t);
        if (ctx->current_rir_op == RIR_TYPE_CATEGORY_COUNT) {
            DD("ctx->current_rir_op == RIR_TYPE_CATEGORY_COUNT "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
            ctx->current_rir_op = n_type_op;
            if (rir_type_op_from_rir_type(current_rir) == RIR_TYPE_CATEGORY_COUNT) {
                DD("rir_type_op_from_rir_type(current_rir) == RIR_TYPE_CATEGORY_COUNT -> return false "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
                return false;
            }
            // also go to the first index of the type
            rir_type_cmp_ctx_idx_plus1(ctx);
        } else {
            DD("ctx->current_rir_op != RIR_TYPE_CATEGORY_COUNT "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
            // check if we need to go deeper into the rir type
            if (n_type_op != ctx->current_rir_op) {
                DD("going deepper into the rir type\n");
                rir_type_cmp_ctx_push_type(ctx, current_rir);

                // also go to the first index of the new type
                ctx->current_rir_op = n_type_op;
                rir_type_cmp_ctx_idx_plus1(ctx);
            }
        }

    } else if (t->category == TYPE_CATEGORY_DEFINED) {
        DD("t->category == TYPE_CATEGORY_DEFINED\n");
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

/**
 * The post callback is called during the comparison traversal "going upwards"
 * in the type. It contains the logic that performs all the basic checks for each
 * leaf element of the type. It also modifies the comparison context accoridngly
 * so that we always know where in the comparison we are located.
 *
 * @param t        The part of the original type we are comparing at this moment
 * @param ctx      The comparison context holding all the necessary information
 * @return         true if comparison can continue, false if we got a comparison failure
 */
bool rir_type_cmp_post_cb(struct type *t, struct rir_type_cmp_ctx *ctx)
{
    const struct rir_type *curr_rir;

    switch (t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        DD("post_cb[elementary] "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
        curr_rir = rir_type_cmp_ctx_current_type(ctx);
        if (!curr_rir) {
            DD("post_cb[elementary] !curr_rir -> return false\n");
            return false;
        }
        if (!rir_type_is_elementary(curr_rir)) {
            DD("post_cb[elementary] !rir_type_is_elementary(curr_rir) -> return false\n");
            return false;
        }
        return (enum elementary_type)curr_rir->category == type_elementary(t);
    case TYPE_CATEGORY_WILDCARD:
        curr_rir = rir_type_cmp_ctx_current_type(ctx);
        if (!curr_rir) {
            DD("post_cb[elementary] !curr_rir -> return false\n");
            return false;
        }
        return curr_rir->category == RIR_TYPE_WILDCARD;

    case TYPE_CATEGORY_DEFINED:
        rir_type_cmp_ctx_go_up(ctx, t);
        break;
    case TYPE_CATEGORY_MODULE:

        break;

    case TYPE_CATEGORY_OPERATOR:
        DD("post_cb[operator] going up\n");
        rir_type_cmp_ctx_go_up(ctx, t);
        break;

    case TYPE_CATEGORY_LEAF:
        DD("post_cb[leaf] going up "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(type_str_or_die(t, TSTR_DEFAULT)));
        curr_rir = rir_type_cmp_ctx_current_type(ctx);
        if (!curr_rir) {
            DD("post_cb[leaf] !curr_rir -> return false\n");
            return false;
        }
        if (!curr_rir->name || !rf_string_equal(curr_rir->name, t->leaf.id)) {
            DD("post_cb[leaf] !curr_rir->name || !rf_string_equal(curr_rir->name, t->leaf.id)r -> return false\n");
            return false;
        }
        if (!rir_type_equals_type(curr_rir, t->leaf.type, NULL)) {
            DD("post_cb[leaf] !rir_type_equals_type(curr_rir, t->leaf.type, NULL) -> return false\n");
            return false;
        }
        // also index + 1
        rir_type_cmp_ctx_idx_plus1(ctx);
        break;
    default:
        RF_CRITICAL_FAIL("Illegal type category for comparison");
        return false;
    }
    return true;
}

bool rir_type_equals_type(const struct rir_type *r_type,
                          const struct type *n_type,
                          const struct RFstring *name)
{
    struct rir_type_cmp_ctx ctx;
    bool ret;

    if (name && (!r_type->name || !rf_string_equal(name, r_type->name))) {
        return false;
    }

    // traverse the normal type, comparing at each step with the rir type
    // for equality.
    rir_type_cmp_ctx_init(&ctx, r_type);
    ret = type_traverse(
        (struct type*)n_type,
        (type_iterate_cb)(rir_type_cmp_pre_cb),
        (type_iterate_cb)(rir_type_cmp_post_cb),
        &ctx
    );
    // if we are at the end of a rir_type operator comparison that has subtypes
    // then check if all of has been covered. If not then it's not a match
    if (ret && darray_top(ctx.indices) != -1 && darray_top(ctx.indices)  != darray_size(rir_type_contents(r_type)->subtypes)) {
        DD("at end of comparison %d != %lu -> return false;\n", darray_top(ctx.indices), darray_size(rir_type_contents(r_type)->subtypes));
        ret = false;
    }
    rir_type_cmp_ctx_deinit(&ctx);
    return ret;
}

const struct rir_type *rir_type_contents(const struct rir_type *t)
{
    return t->category == COMPOSITE_RIR_DEFINED ? darray_item(t->subtypes, 0) : t;
}

const struct RFstring *rir_type_get_nth_name(const struct rir_type *t, unsigned n)
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
i_INLINE_INS const struct RFstring *rir_type_get_nth_name_or_die(const struct rir_type *t, unsigned n);

const struct rir_type *rir_type_get_nth_type(const struct rir_type *t, unsigned n)
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
i_INLINE_INS const struct rir_type *rir_type_get_nth_type_or_die(const struct rir_type *t, unsigned n);

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
        RF_CRITICAL_FAIL("Invalid rir_type at rir_type_op_to_str()");
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
        RF_CRITICAL_FAIL("Invalid rir_type at rir_type_str()");
        break;
    }
    return NULL;
}
i_INLINE_INS struct RFstring *rir_type_str_or_die(const struct rir_type *t);

// NOTE: preserve order
static struct rir_type i_rir_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)    \
    [i_type] = {                                    \
        .category = (enum rir_type_category)i_type, \
        .type = NULL,                               \
        .name = NULL,                               \
    }
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};

const struct rir_type *rir_type_get_elementary(enum elementary_type etype)
{
    return &i_rir_elementary_types[etype];
}

bool rir_type_statically_alloced(struct rir_type *t)
{
    if (t >= &i_rir_elementary_types[0] && t <= &i_rir_elementary_types[ELEMENTARY_TYPE_NIL] + sizeof(struct rir_type)) {
        return true;
    }
    return false;
}

i_INLINE_INS bool rir_type_is_elementary(const struct rir_type *t);
i_INLINE_INS bool rir_type_is_true_elementary(const struct rir_type *t);
i_INLINE_INS bool rir_type_is_category(const struct rir_type *t,
                                       enum rir_type_category category);
i_INLINE_INS bool rir_type_is_operator(const struct rir_type *t);
i_INLINE_INS bool rir_type_is_trivial(const struct rir_type *t);
i_INLINE_INS const struct type *rir_type_get_type_or_die(const struct rir_type *type);

size_t rir_type_bytesize(const struct rir_type *t)
{
    // TODO
    return 4;
}
