#include <types/type_arr.h>

#include <rfbase/utils/memory.h>

#include <types/type.h>
#include <module.h>

#include <ast/ast.h>
#include <ast/constants.h>

void type_array_init(
    struct type *tarr,
    const struct type *member_type,
    struct arr_int64 *dimensions)
{
    tarr->category = TYPE_CATEGORY_ARRAY;
    tarr->is_constant = false;
    darray_shallow_copy(tarr->array.dimensions, *dimensions);
    tarr->array.member_type = member_type;
}

struct type *type_array_get_or_create_from_ast(
    struct module *mod,
    struct ast_node *astarr,
    const struct type *member_type)
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
    return module_getorcreate_type_as_arr(mod, member_type, &dimensions);
}

void type_array_destroy(struct type *tarr)
{
    darray_free(tarr->array.dimensions);
}

i_INLINE_INS const struct type * type_array_member_type(const struct type *t);
i_INLINE_INS int64_t type_get_arr_first_size(const struct type *t);
i_INLINE_INS bool type_is_elementary_array(const struct type *t);

const struct RFstring* type_array_specifier_str(const struct arr_int64 *dims)
{
    struct RFstring *ret = RFS("");
    int64_t *val;
    darray_foreach(val, *dims) {
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
    const struct arr_int64 *dimensions
);

struct type *module_getorcreate_type_as_arr(
    struct module *mod,
    const struct type *t,
    struct arr_int64 *dimensions)
{
    RFS_PUSH();
    struct type *found_type = module_types_set_has_str(
        mod,
        type_str_add_array(type_str(t, TSTR_DEFAULT), dimensions)
    );
    RFS_POP();
    if (!found_type) {
        // if not found, we gotta create it and add it
        if (!(found_type = type_alloc(mod))) {
            RF_ERROR("Failed to create a type");
            return NULL;
        }
        type_array_init(found_type, t, dimensions);
        module_types_set_add(mod, found_type, NULL);
    } else {
        darray_free(*dimensions);
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
    return module_getorcreate_type_as_arr(mod, t, &dims);
}
