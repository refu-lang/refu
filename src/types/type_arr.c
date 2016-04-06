#include <types/type_arr.h>

#include <rflib/utils/memory.h>

#include <types/type.h>
#include <module.h>

#include <ast/ast.h>
#include <ast/constants.h>

void type_arr_init(struct type_arr *arr, struct arr_int64 *dimensions)
{
    darray_shallow_copy(arr->dimensions, *dimensions);
}

struct type_arr *type_arr_create(struct arr_int64 *dimensions)
{
    struct type_arr *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    type_arr_init(ret, dimensions);
    return ret;
}

struct type_arr *type_arr_create_from_ast(struct ast_node *astarr)
{
    AST_NODE_ASSERT_TYPE(astarr, AST_ARRAY_SPEC);
    struct arr_int64 dimensions;
    darray_init(dimensions);
    struct ast_node **dimension;
    darray_foreach(dimension, astarr->children) {
        if (*dimension == ast_node_placeholder()) {
            darray_append(dimensions, -1);
        } else {
            AST_NODE_ASSERT_TYPE(*dimension, AST_CONSTANT);
            int64_t value;
            if (!ast_constant_get_integer(&(*dimension)->constant, &value)) {
                return NULL;
            }
            darray_append(dimensions, value);
        }
    }
    return type_arr_create(&dimensions);
}

void type_arr_destroy(struct type_arr *arr)
{
    darray_free(arr->dimensions);
    free(arr);
}

bool type_arr_equal(const struct type_arr *a1, const struct type_arr *a2)
{
    if (!a1 && !a2) {
        return true;
    }

    if (!!a1 ^ !!a2) {
        return false;
    }

    if (darray_size(a1->dimensions) != darray_size(a2->dimensions)) {
        return false;
    }

    int64_t *fromval;
    unsigned i = 0;
    darray_foreach(fromval, a1->dimensions) {
        int64_t toval = darray_item(a2->dimensions, i++);
        if (*fromval != toval && *fromval != -1 && toval != -1) {
            return false;
        }
    }
    return true;
}

const struct RFstring* type_arr_str(const struct type_arr *arr)
{
    struct RFstring *ret = RFS("");
    int64_t *val;
    darray_foreach(val, arr->dimensions) {
        if (*val == -1) {
            ret = RFS(RFS_PF "[]", RFS_PA(ret));
        } else {
            ret = RFS(RFS_PF "[%" PRId64 "]", RFS_PA(ret), *val);
        }
    }
    return ret;
}

i_INLINE_INS struct RFstring *type_str_add_array(
    struct RFstring *str,
    const struct type_arr *arr
);

struct type *module_getorcreate_type_as_arr(
    struct module *mod,
    const struct type *t,
    struct type_arr *arrtype)
{
    RFS_PUSH();
    struct type *found_type = module_types_set_has_str(
        mod,
        type_str_add_array(type_str(t, TSTR_DEFAULT), arrtype)
    );
    RFS_POP();
    if (!found_type) {
        // if not found, we gotta create it and add it
        if (!(found_type = type_alloc_copy(mod, t))) {
            RF_ERROR("Failed to create a copy of a type");
            return NULL;
        }
        found_type->array = arrtype;
        module_types_set_add(mod, found_type, NULL);
    } else {
        type_arr_destroy(arrtype);
    }
    return found_type;
}

struct type *module_getorcreate_type_as_singlearr(
    struct module *mod,
    const struct type *t,
    int64_t dimension)
{
    struct arr_int64 dims;
    darray_init(dims);
    darray_append(dims, dimension);

    struct type_arr *arrtype = type_arr_create(&dims);
    return module_getorcreate_type_as_arr(mod, t, arrtype);
}
