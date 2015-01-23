#include <ir/elements.h>
#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>
#include <ast/type_decls.h>
#include <String/rf_str_decl.h>
#include <String/rf_str_core.h>

static struct rir_type i_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)                           \
    [i_type] = {.elementary = i_type}

    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};


bool rir_type_init(struct rir_type *type, struct type *input)
{
    struct rir_type *left_type_ptr;
    struct rir_type *right_type_ptr;
    darray_init(type->subtypes);

    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_GENERIC:
        break;

    case TYPE_CATEGORY_OPERATOR:
        // TODO: fix this
        if (input->operator.type != TYPEOP_PRODUCT) {
            RF_ASSERT(false, "Sum types not yet supported at this stage");
        }

        if (!(left_type_ptr = rir_type_create(input->operator.left))) {
            return false;
        }
        darray_push(type->subtypes, left_type_ptr);

        if (!(right_type_ptr = rir_type_create(input->operator.right))) {
            rir_type_destroy(left_type_ptr);
            return false;
        }
        darray_push(type->subtypes, right_type_ptr);
        break;

    case TYPE_CATEGORY_LEAF:
        if (!rir_type_init(type, input->leaf.type)) {
            return false;
        }
        break;
    }

    return true;
}

struct rir_type *rir_type_alloc(struct type *input)
{
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        return &i_elementary_types[type_elementary(input)];
    }

    // else
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    return ret;

}

struct rir_type *rir_type_create(struct type *input)
{
    struct rir_type *ret = rir_type_alloc(input);
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }

    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    // TODO: If t is not elementary then free
    (void)t;
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
        rir_type_destroy(*subtype);
    }

    darray_free(t->subtypes);
}

/* -- rir_function -- */

RF_STRUCT_INIT_SIG(rir_function, struct type* fn_type, struct RFstring *name)
{
    if (!rf_string_copy_in(&this->name, name)) {
        RF_ERROR("failed to iniailize rir_function name");
        return false;
    }

    this->arg_type = rir_type_create(type_function_get_argtype(fn_type));
    if (!this->arg_type) {
        RF_ERROR("Failed to create rir_function argument type");
        return false;
    }

    this->ret_type = rir_type_create(type_function_get_rettype(fn_type));
    if (!this->ret_type) {
        RF_ERROR("Failed to create rir_function return type");
        return false;
    }

    return true;
}

RF_STRUCT_DEINIT_SIG(rir_function)
{
    rf_string_deinit(&this->name);
    rir_type_destroy(this->arg_type);
    rir_type_destroy(this->ret_type);
}

RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_function, struct type*, fn_type,
                               struct RFstring*, name)
