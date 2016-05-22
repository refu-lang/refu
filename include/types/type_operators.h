#ifndef LFR_TYPES_TYPE_OPERATORS_H
#define LFR_TYPES_TYPE_OPERATORS_H

#include <rfbase/utils/sanity.h>
#include <rfbase/defs/inline.h>
#include <rfbase/string/decl.h>

#include <types/type_decls.h>

/**
 * Add a type as a subtype operand of a type operator
 */
void type_operator_add_operand(struct type_operator *p, struct type *c);
i_INLINE_DECL void type_add_operand(struct type *p, struct type *c)
{
    RF_ASSERT(p->category == TYPE_CATEGORY_OPERATOR, "Expected a type operator");
    return type_operator_add_operand(&p->operator, c);
}

i_INLINE_DECL bool type_is_operator(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR;
}

i_INLINE_DECL struct type *typeop_to_type(struct type_operator *op)
{
    return container_of(op, struct type, operator);
}

/**
 * Query a type's operator type. If type is not an operator then TYPEOP_INVALID is returned
 */
i_INLINE_DECL enum typeop_type type_typeop_get(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR ? t->operator.type : TYPEOP_INVALID;
}

/**
 * Query if a type is a sum operator type
 */
i_INLINE_DECL bool type_is_sumop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_SUM;
}

/**
 * Query if a type is a product operator type
 */
i_INLINE_DECL bool type_is_prodop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_PRODUCT;
}

i_INLINE_DECL bool type_is_implop(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_IMPLICATION;
}

/**
 * Query if a type is either a sum operator type or a defined type which contains
 * a sum of other types
 */
i_INLINE_DECL bool type_is_sumtype(const struct type *t)
{
    return type_is_sumop(t) ||
        (t->category == TYPE_CATEGORY_DEFINED && type_is_sumop(t->defined.type));
}

/**
 * @return the nth subtype of a type operation, or NULL if it does not exist or
 * if the type is not a type operator
 */
const struct type *type_get_subtype(const struct type *t, unsigned int index);

/**
 * @return the number of operands/subtypes a type operation has
 */
unsigned int type_get_subtypes_num(const struct type *t);

/**
 * @return The index of @a t inside @a maybe_parent if it's found and -1 if not
 */
int type_is_direct_childof(const struct type *t, const struct type *maybe_parent);

/**
 * Works just like @ref type_is_direct_childof() but also allows for @a to be a
 * defined type
 */
i_INLINE_DECL int type_is_childof(const struct type *t, const struct type *maybe_parent)
{
    return maybe_parent->category == TYPE_CATEGORY_DEFINED
        ? type_is_direct_childof(t, maybe_parent->defined.type)
        : type_is_direct_childof(t, maybe_parent);
}
#endif
