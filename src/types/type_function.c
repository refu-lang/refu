#include <types/type_function.h>

#include <module.h>
#include <types/type.h>
#include <String/rf_str_core.h>

i_INLINE_INS bool type_is_function(const struct type *t);
i_INLINE_INS bool type_is_callable(const struct type *t);
i_INLINE_INS struct type *type_callable_get_argtype(const struct type *t);
i_INLINE_INS struct type *type_callable_get_rettype(const struct type *t);

static const struct RFstring s_function_ = RF_STRING_STATIC_INIT("function");
static const struct RFstring s_ctor_ = RF_STRING_STATIC_INIT("constructor");

struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_function(t) && darray_size(t->operator.operands) == 2,
              "Non function type detected");
    return darray_item(t->operator.operands, 0);
}

struct type *type_function_get_rettype(const struct type *t)
{
    RF_ASSERT(type_is_function(t) && darray_size(t->operator.operands) == 2,
              "Non function type detected");
    return darray_item(t->operator.operands, 1);
}

const struct RFstring *type_callable_category_str(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return &s_function_;
    }
    return &s_ctor_;
}

static inline bool type_is_product_op(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_PRODUCT;
}

void type_function_init(struct type *t, struct type *arg_type, struct type *ret_type)
{
    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = TYPEOP_IMPLICATION;

    darray_append(t->operator.operands, arg_type);
    darray_append(t->operator.operands, ret_type);
}
