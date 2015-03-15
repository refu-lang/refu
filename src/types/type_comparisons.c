#include <types/type_comparisons.h>

#include <Utils/sanity.h>
#include <String/rf_str_core.h>

#include <ast/ast.h>
#include <ast/type.h>

#include <types/type.h>
#include <types/type_elementary.h>

i_INLINE_INS void type_comparison_ctx_init(struct type_comparison_ctx *ctx,
                                           enum comparison_reason reason);
i_INLINE_INS enum comparison_reason type_comparison_ctx_reason(struct type_comparison_ctx *ctx);


i_INLINE_INS bool type_category_equals(const struct type* t,
                                       enum type_category category);

static inline bool type_leaf_equals(const struct type_leaf *t1,
                                    const struct type_leaf *t2,
                                    struct type_comparison_ctx *ctx)
{
    bool predicate = true;
    if (type_comparison_ctx_reason(ctx) == COMPARISON_REASON_IDENTICAL) {
        predicate = rf_string_equal(t1->id, t2->id);
    }
    return predicate && type_equals(t1->type, t2->type, ctx);
}

static bool type_operator_equals(const struct type_operator *t1,
                                 const struct type_operator *t2,
                                 struct type_comparison_ctx *ctx)
{
    if (t1 == t2) {
        return true;
    }

    if (t1->type != t2->type) {
        return false;
    }

    return type_equals(t1->left, t2->left, ctx) &&
           type_equals(t1->right, t2->right, ctx);
}

static bool type_same_categories_equals(const struct type* t1,
                                        const struct type *t2,
                                        struct type_comparison_ctx *ctx)
{
    RF_ASSERT(t1->category == t2->category, "type_same_categories_equals() called "
              "with types of different categories");
    switch (t1->category) {
    case TYPE_CATEGORY_OPERATOR:
        return type_operator_equals(&t1->operator, &t2->operator, ctx);
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_equals(&t1->elementary, &t2->elementary, ctx);
    case TYPE_CATEGORY_LEAF:
            return type_leaf_equals(&t1->leaf, &t2->leaf, ctx);
    case TYPE_CATEGORY_DEFINED:
        return rf_string_equal(t1->defined.name, t2->defined.name) &&
               type_equals(t1->defined.type, t2->defined.type, ctx);
    case TYPE_CATEGORY_GENERIC:
        //TODO
        RF_ASSERT(false, "Not yet implemented");
        break;
    default:
        RF_ASSERT(false, "Illegal type category");
        break;
    }
    return false;
}

//! Possible resuls of @see type_initial_check()
enum type_initial_check_result {
    TYPES_ARE_NOT_EQUAL,
    TYPES_ARE_EQUAL,
    TYPES_CHECK_CAN_CONTINUE,
};
/**
 *  Performs the first step of type comparison making sure that after its call
 *  one of 3 things happen:
 *
 *  * @c TYPES_ARE_NOT_EQUAL:          The comparison fails immediately
 *  * @c TYPES_ARE_EQUAL:              The comparison succeeds
 *  * @c TYPES_CHECK_CAN_CONTINUE:     Made sure compared types are of same category
 *                                     and type equality comparison can proceed.
 */
static inline enum type_initial_check_result type_category_check(const struct type *t1,
                                                                 const struct type *t2,
                                                                 struct type_comparison_ctx *ctx)
{
    enum type_initial_check_result ret = TYPES_ARE_NOT_EQUAL;

    if (t1->category == t2->category) {
        ret = TYPES_CHECK_CAN_CONTINUE;
    }

    // A type should be equal to a leaf of the same type and to a defined of the same type
    // but should not be considered identical to it
    if (type_comparison_ctx_reason(ctx) != COMPARISON_REASON_IDENTICAL) {
        if (t1->category == TYPE_CATEGORY_ELEMENTARY && t2->category == TYPE_CATEGORY_LEAF) {
            if (type_same_categories_equals(t1, t2->leaf.type, ctx)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (t2->category == TYPE_CATEGORY_ELEMENTARY && t1->category == TYPE_CATEGORY_LEAF) {
            if (type_same_categories_equals(t1->leaf.type, t2, ctx)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (t1->category == TYPE_CATEGORY_DEFINED) {
            if (type_equals(t1->defined.type, t2, ctx)) {
                ret = TYPES_ARE_EQUAL;
            }
        } else if (t2->category == TYPE_CATEGORY_DEFINED) {
            if (type_equals(t1, t2->defined.type, ctx)) {
                ret = TYPES_ARE_EQUAL;
            }
        }
    }

    return ret;
}

bool type_equals(const struct type* t1, const struct type *t2,
                 struct type_comparison_ctx *ctx)
{
    // first check if we refer to the same type (elementary or composite)
    if (t1 == t2) {
        if (ctx) {
            ctx->common_type = t1;
        }
        return true;
    }

    switch (type_category_check(t1, t2, ctx)) {
    case TYPES_ARE_EQUAL:
        if (ctx) {
            ctx->common_type = t1;
        }
        return true;
    case TYPES_ARE_NOT_EQUAL:
        return false;
    case TYPES_CHECK_CAN_CONTINUE:
        break;
    }

    // from here and on we must have same categories of types
    return type_same_categories_equals(t1, t2, ctx);
}

bool type_equals_ast_node(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl)
{
    struct type *looked_up_t;
    struct type_comparison_ctx cmp_ctx;
    type_comparison_ctx_init(&cmp_ctx, COMPARISON_REASON_GENERIC);
    switch(type_desc->type) {
    case AST_TYPE_OPERATOR:
        if (t->category != TYPE_CATEGORY_OPERATOR) {
            return false;
        }

        if (t->operator.type != ast_typeop_op(type_desc)) {
            return false;
        }

        return type_equals_ast_node(t->operator.left,
                                    ast_typeop_left(type_desc),
                                    a, st, genrdecl) &&
            type_equals_ast_node(t->operator.right,
                                 ast_typeop_right(type_desc),
                                 a, st, genrdecl);
    case AST_TYPE_DESCRIPTION:
    {
        AST_NODE_ASSERT_TYPE(ast_typedesc_left(type_desc), AST_IDENTIFIER);
        bool predicate = true;
        if (t->category == TYPE_CATEGORY_LEAF) {
            predicate = rf_string_equal(t->leaf.id, ast_identifier_str(ast_typedesc_left(type_desc)));
        }
        return predicate && type_equals_ast_node(t, ast_typedesc_right(type_desc),
                                                 a, st, genrdecl);
    }
    case AST_TYPE_DECLARATION:
        return type_equals_ast_node(t, ast_typedecl_typedesc_get(type_desc),
                                    a, st, genrdecl);
    case AST_XIDENTIFIER:
        looked_up_t = type_lookup_xidentifier(type_desc, a, st, genrdecl);
        if (!looked_up_t) {
            RF_ERROR("Failed to lookup an identifier");
            return false;
        }

        return type_equals(t, looked_up_t, &cmp_ctx);
    default:
        break;
    }

    RF_ASSERT_OR_CRITICAL(false, "Illegal ast node type \""RF_STR_PF_FMT"\""
                          " detected instead of a type description",
                          RF_STR_PF_ARG(ast_node_str(type_desc)));
    return false;
}
