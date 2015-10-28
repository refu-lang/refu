#include <types/type_operators.h>
#include <types/type_comparisons.h>

void type_operator_add_operand(struct type_operator *p, struct type *c)
{
    if (p != &c->operator) {
        darray_append(p->operands, c);
    }
}
i_INLINE_INS void type_add_operand(struct type *p, struct type *c);

i_INLINE_INS bool type_is_operator(const struct type *t);
i_INLINE_INS struct type *typeop_to_type(struct type_operator *op);
i_INLINE_INS enum typeop_type type_typeop_get(const struct type *t);
i_INLINE_INS bool type_is_sumop(const struct type *t);
i_INLINE_INS bool type_is_prodop(const struct type *t);
i_INLINE_INS bool type_is_implop(const struct type *t);
i_INLINE_INS bool type_is_sumtype(const struct type *t);

const struct type *type_get_subtype(const struct type *t, unsigned int index)
{
    if (t->category != TYPE_CATEGORY_OPERATOR) {
        return NULL;
    }
    if (index >= darray_size(t->operator.operands)) {
        return NULL;
    }
    return darray_item(t->operator.operands, index);
}

unsigned int type_get_subtypes_num(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_OPERATOR, "Can't get operand of non-operator type");
    return darray_size(t->operator.operands);
}

int type_is_direct_childof(const struct type *t, const struct type *maybe_parent)
{
    if (maybe_parent->category != TYPE_CATEGORY_OPERATOR) {
        return false;
    }
    struct type **subt;
    int idx = 0;
    darray_foreach(subt, maybe_parent->operator.operands) {
        if (type_compare(*subt, t, TYPECMP_IDENTICAL)) {
            return idx;
        }
        ++idx;
    }
    return -1;
}
i_INLINE_INS int type_is_childof(const struct type *t, const struct type *maybe_parent);
