#include <types/type_function.h>

i_INLINE_INS struct type *type_function_get_argtype(const struct type *t);
i_INLINE_INS struct type *type_function_get_rettype(const struct type *t);

static const struct type *do_type_function_get_argtype_n(const struct type *t, unsigned int n)
{
    // TODO: This should probably be an actual check and not an assert
    RF_ASSERT(t->category == TYPE_CATEGORY_ANONYMOUS &&
              t->anonymous.is_operator &&
              t->anonymous.op.type == TYPEOP_PRODUCT,
              "Unexpected function argument format");

    return (n == 0) ? t->anonymous.op.left
                    : do_type_function_get_argtype_n(t->anonymous.op.right, n -1);
}

const struct type *type_function_get_argtype_n(const struct type *t, unsigned int n)
{
    // special case of 1 argument only
    if (t->category == TYPE_CATEGORY_LEAF) {
        if (n != 0) {
            return NULL;
        }

        return t;
    }

    // else it should be a product operator, since arguments should be comma separated
    // TODO: This should probably be an actual check and not an assert
    RF_ASSERT(t->category == TYPE_CATEGORY_ANONYMOUS &&
              t->anonymous.is_operator &&
              t->anonymous.op.type == TYPEOP_PRODUCT,
              "Unexpected function argument format");

    return do_type_function_get_argtype_n(t, n);
}
