#include <types/type_arr.h>

#include <rflib/utils/memory.h>

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

    int64_t *val;
    unsigned i = 0;
    darray_foreach(val, a1->dimensions) {
        if (*val != darray_item(a2->dimensions, i++)) {
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
            ret = RFS(RFS_PF " []", RFS_PA(ret));
        } else {
            ret = RFS(RFS_PF " [" PRId64 "]", RFS_PA(ret), *val);
        }
    }
    return ret;
}

i_INLINE_INS struct RFstring *type_str_add_array(
    struct RFstring *str,
    const struct type_arr *arr
);
