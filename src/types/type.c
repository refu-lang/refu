#include <types/type.h>

#include <Persistent/buffers.h>
#include <Utils/fixed_memory_pool.h>

#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <ast/ast.h>
#include <ast/type.h>
#include <ast/generics.h>
#include <ast/vardecl.h>
#include <ast/function.h>

#include <types/type_elementary.h>
#include <types/type_function.h>

/* -- forward declarations of functions -- */
static bool type_operator_init(struct type_operator *t, struct ast_node *n,
                                struct analyzer *a, struct symbol_table *st,
                                struct ast_node *genrdecl);

/* -- miscellaneous type functions used internally (static) -- */
struct function_args_ctx {
    struct symbol_table *st;
    struct analyzer *analyzer;
};

static bool do_type_function_add_args_to_st(struct type_leaf *lt, void *user)
{
    struct function_args_ctx *ctx = user;
    return symbol_table_add_type(ctx->st, ctx->analyzer, lt->id, lt->type);
}

static bool type_function_add_args_to_st(struct type *args_t,
                                         struct analyzer *a,
                                         struct symbol_table *st)
{
    struct function_args_ctx ctx;
    ctx.st = st;
    ctx.analyzer = a;
    return type_for_each_leaf(args_t, do_type_function_add_args_to_st, &ctx);
}


/* -- type allocation functions -- */

struct type *type_alloc(struct analyzer *a)
{
    return rf_fixed_memorypool_alloc_element(a->types_pool);
}

void type_free(struct type *t, struct analyzer *a)
{
    rf_fixed_memorypool_free_element(a->types_pool, t);
}

/* -- type creation and initialization functions used internally -- */

static bool type_leaf_init(struct type_leaf *leaf, struct ast_node *typedesc,
                           struct analyzer *a, struct symbol_table *st,
                           struct ast_node *genrdecl)
{
    struct ast_node *right;
    struct ast_node *left;
    right = ast_typedesc_right(typedesc);
    left = ast_typedesc_left(typedesc);

    AST_NODE_ASSERT_TYPE(left, AST_IDENTIFIER);
    leaf->id = ast_identifier_str(left);

    if (right->type == AST_XIDENTIFIER) {
        leaf->type = type_lookup_xidentifier(right, a, st, genrdecl);
        if (!leaf->type) {
            RF_ERROR("No type could be looked up for xidentifier \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_xidentifier_str(right)));
            return false;
        }

    } else if (right->type == AST_TYPE_DESCRIPTION ||
               right->type == AST_TYPE_OPERATOR) {
        leaf->type = analyzer_get_or_create_type(a, right, st, genrdecl);
        if (!leaf->type) {
            return false;
        }

    } else {
        RF_ASSERT_OR_CRITICAL(false, "Illegal ast node type \""RF_STR_PF_FMT"\""
                              " detected as the right part of a type description",
                              RF_STR_PF_ARG(ast_node_str(right)));
        return false;
    }

    return true;
}

static bool type_init_from_typedesc(struct type *t, struct ast_node *typedesc,
                                    struct analyzer *a, struct symbol_table *st,
                                    struct ast_node *genrdecl)
{
    if (ast_types_left(typedesc)->type == AST_IDENTIFIER) {
        AST_NODE_ASSERT_TYPE(typedesc, AST_TYPE_DESCRIPTION);

        t->category = TYPE_CATEGORY_LEAF;
        return type_leaf_init(&t->leaf, typedesc, a, st, genrdecl);
    } else {
        t->category = TYPE_CATEGORY_OPERATOR;
        return type_operator_init(&t->operator, typedesc, a, st, genrdecl);
    }
}

struct type *type_create_from_typedesc(struct ast_node *typedesc,
                                       struct analyzer *a,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl)
{
    struct type *ret;
    ret = type_alloc(a);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    if (!type_init_from_typedesc(ret, typedesc, a, st, genrdecl)) {
        type_free(ret, a);
        return NULL;
    }
    return ret;
}

static bool type_operator_init(struct type_operator *t, struct ast_node *n,
                               struct analyzer *a, struct symbol_table *st,
                               struct ast_node *genrdecl)
{
    struct type *left;
    struct type *right;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);

    // TODO: Figure out why type_lookup_or_create() fails the tests if used here
    t->type = ast_typeop_op(n);
    left = type_create(ast_typeop_left(n), a, st, genrdecl);
    if (!left) {
        return false;
    }
    right = type_create(ast_typeop_right(n), a, st, genrdecl);
    if (!right) {
        return false;
    }

    t->left = left;
    t->right = right;

    return true;
}

struct type *type_lookup_or_create(struct ast_node *n,
                                   struct analyzer *a,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl)
{
    switch (n->type) {
    case AST_XIDENTIFIER:
        // lookup the xidentifier itself
        return type_lookup_xidentifier(n, a, st, genrdecl);
    case AST_TYPE_DESCRIPTION:
        // if it's a simple type description (e.g. only identifier COLON xidentifier
        if (ast_typedesc_left(n)->type == AST_IDENTIFIER &&
            ast_typedesc_right(n)->type == AST_XIDENTIFIER) {
            return type_lookup_xidentifier(ast_typedesc_right(n), a, st, genrdecl);
        }
        // else fall through case, same as type operator
    case AST_TYPE_OPERATOR:
        return analyzer_get_or_create_type(a, n, st, genrdecl);
    default:
        RF_ASSERT_OR_CRITICAL(false,"Unexpected ast node type "
                              "\""RF_STR_PF_FMT"\" detected",
                              RF_STR_PF_ARG(ast_node_str(n)));
    break;

    }

    return NULL;
}

struct type *type_create_from_operation(enum typeop_type type,
                                        struct type *left,
                                        struct type *right,
                                        struct analyzer *a)
{
    struct type *t;
    // TODO: Somehow also check if the type is already existing in the analyzer
    //       and it is return it instead of creating it
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = type;
    t->operator.left = left;
    t->operator.right = right;

    return t;
}

/* -- various type creation and initialization functions -- */

struct type *type_create(struct ast_node *node,
                         struct analyzer *a, struct symbol_table *st,
                         struct ast_node *genrdecl)
{
    switch (node->type) {
    case AST_TYPE_DECLARATION:
        return type_create_from_typedecl(node, a, st);
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_OPERATOR:
        return type_create_from_typedesc(node, a, st, genrdecl);
    case AST_FUNCTION_DECLARATION:
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, "Attempted to create a type "
                              "for illegal ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(node)));
        break;
    }

    return NULL;
}

struct type *type_create_from_typedecl(struct ast_node *n,
                                       struct analyzer *a,
                                       struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);

    return type_create_from_typedesc(ast_typedecl_typedesc_get(n),
                                     a,
                                     st,
                                     ast_typedecl_genrdecl_get(n));

}

struct type *type_create_from_vardecl(struct ast_node *n,
                                      struct analyzer *a,
                                      struct symbol_table *st)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);

    t = type_lookup_or_create(ast_vardecl_desc_get(n), a, st, NULL);
    if (!t) {
        return NULL;
    }

    return t;
}

static bool type_init_from_fndecl(struct type *t,
                                  struct ast_node *n,
                                  struct analyzer *a,
                                  struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    struct ast_node *args = ast_fndecl_args_get(n);
    struct ast_node *ret = ast_fndecl_return_get(n);
    struct type *arg_type = NULL;
    struct type *ret_type = NULL;

    // set argument type (left part of the operand)
    if (args) {
        arg_type = type_lookup_or_create(args, a, st,
                                         ast_fndecl_genrdecl_get(n));
        if (!arg_type) {
            // TODO: Free argument_type if created
            return false;
        }
        // also add the function's arguments to its symbol table
        type_function_add_args_to_st(arg_type, a, ast_fndecl_symbol_table_get(n));
    }

    if (ret) {
        ret_type = type_lookup_or_create(ret, a, st,
                                         ast_fndecl_genrdecl_get(n));
        if (!ret_type) {
            // TODO: Free return_type if created
            return false;
        }
    }

    type_function_init(t, arg_type, ret_type);

    return true;
}

struct type *type_create_from_fndecl(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);

    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    if (!type_init_from_fndecl(t, n, a, st)) {
        type_free(t, a);
        t = NULL;
    }

    return t;
}

struct type *type_operator_create(struct ast_node *n,
                                  struct analyzer *a,
                                  struct symbol_table *st,
                                  struct ast_node *genrdecl)
{
    struct type *t;
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    t->category = TYPE_CATEGORY_OPERATOR;
    if (!type_operator_init(&t->operator, n, a, st, genrdecl)) {
        type_free(t, a);
        t = NULL;
    }

    return t;
}

/* -- type comparison functions -- */

i_INLINE_INS void type_comparison_ctx_init(struct type_comparison_ctx *ctx,
                                           enum comparison_reason reason);

//! Possible resuls of @see type_initial_check()
enum type_initial_check_result {
    TYPES_ARE_NOT_EQUAL,
    TYPES_ARE_EQUAL,
    TYPES_CHECK_CAN_CONTINUE,
};
/**
 *  Performs the first step of type comparison with 3 possible outcomes
 */
static inline enum type_initial_check_result type_initial_check(const struct type *t1,
                                                                const struct type *t2,
                                                                struct type_comparison_ctx *ctx)
{
    enum type_initial_check_result ret = TYPES_ARE_NOT_EQUAL;

    if (t1->category == t2->category) {
        ret = TYPES_CHECK_CAN_CONTINUE;
    }

    // TODO: This should be also changed to include user defined types and not only elementary ones
    // A type should be equal to a leaf of the same type
    if (t1->category == TYPE_CATEGORY_ELEMENTARY && t2->category == TYPE_CATEGORY_LEAF) {
        if (type_equals(t1, t2->leaf.type, ctx)) {
            ret = TYPES_ARE_EQUAL;
        }
    } else if (t2->category == TYPE_CATEGORY_ELEMENTARY && t1->category == TYPE_CATEGORY_LEAF) {
        if (type_equals(t2, t1->leaf.type, ctx)) {
            ret = TYPES_ARE_EQUAL;
        }
    }

    return ret;
}

static inline bool type_leaf_equals(const struct type_leaf *t1,
                                    const struct type_leaf *t2,
                                    struct type_comparison_ctx *ctx)
{
    return type_equals(t1->type, t2->type, ctx);
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

bool type_equals(const struct type* t1, const struct type *t2,
                 struct type_comparison_ctx *ctx)
{
    // first check if we refer to the same type (elementary or composite)
    if (t1 == t2) {
        return true;
    }

    switch (type_initial_check(t1, t2, ctx)) {
    case TYPES_ARE_EQUAL:
        return true;
    case TYPES_ARE_NOT_EQUAL:
        return false;
    case TYPES_CHECK_CAN_CONTINUE:
        break;
    }

    switch (t1->category) {
    case TYPE_CATEGORY_OPERATOR:
        return type_operator_equals(&t1->operator, &t2->operator, ctx);
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_equals(&t1->elementary, &t2->elementary, ctx);
    case TYPE_CATEGORY_LEAF:
            return type_leaf_equals(&t1->leaf, &t2->leaf, ctx);
    case TYPE_CATEGORY_GENERIC:
        //TODO
        RF_ASSERT(false, "Not yet implemented");
        break;
    }

    return false;
}

bool type_equals_ast_node(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl)
{
    if (type_desc->type == AST_TYPE_OPERATOR) {
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

    } else if (type_desc->type == AST_TYPE_DESCRIPTION) {
        AST_NODE_ASSERT_TYPE(ast_typedesc_left(type_desc), AST_IDENTIFIER);
        return type_equals_ast_node(t, ast_typedesc_right(type_desc),
                                    a, st, genrdecl);

    } else if (type_desc->type == AST_XIDENTIFIER) {
        struct type *looked_up_t;
        looked_up_t = type_lookup_xidentifier(type_desc, a, st, genrdecl);
        if (!looked_up_t) {
            RF_ERROR("Failed to lookup an identifier");
            return false;
        }

        return type_equals(t, looked_up_t, NULL);
    } else {
        RF_ASSERT_OR_CRITICAL(false, "Illegal ast node type \""RF_STR_PF_FMT"\""
                              " detected instead of a type description",
                              RF_STR_PF_ARG(ast_node_str(type_desc)));
        return false;
    }

}

/* -- type getters -- */
struct type *type_lookup_xidentifier(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl)
{
    const struct RFstring *id;
    struct type* ret;

    AST_NODE_ASSERT_TYPE(n, AST_XIDENTIFIER);
    id = ast_xidentifier_str(n);

    ret = type_lookup_identifier_string(id, st);
    if (ret) {
        return ret;
    }

    // if not check if we have generic and if it is one of them
    if (genrdecl) {
        struct ast_node *genrtype;
        genrtype = ast_genrdecl_string_is_genr(genrdecl, id);
        if (genrtype) {
            // TODO: read the generic type ast_node and create a generic type
            RF_ASSERT(false, "TODO: Not yet implemented");
            return NULL;
        }
    }

    // if we get here the type can't be recognized
    analyzer_err(a, ast_node_startmark(n),
                 ast_node_endmark(n),
                 "Type \""RF_STR_PF_FMT"\" is not defined",
                 RF_STR_PF_ARG(id));
    return NULL;

}
struct type *type_lookup_identifier_string(const struct RFstring *str,
                                           struct symbol_table *st)
{
    struct symbol_table_record *rec;
    bool at_first_st;
    int elementary_type;

    // check if it's an elementary type
    elementary_type = type_elementary_identifier_p(str);
    if (elementary_type != -1) {
        return (struct type*)type_elementary_get_type(elementary_type);
    }

    // if not check if we know about it from the symbol tables
    rec = symbol_table_lookup_record(st, str, &at_first_st);
    if (rec) {
        return symbol_table_record_type(rec);
    }

    return NULL;
}

const struct RFstring *type_str(const struct type *t)
{
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_get_str(t->elementary.etype);
    case TYPE_CATEGORY_OPERATOR:
        return RFS_(RF_STR_PF_FMT RF_STR_PF_FMT RF_STR_PF_FMT,
                    RF_STR_PF_ARG(type_str(t->operator.left)),
                    RF_STR_PF_ARG(type_op_str(t->operator.type)),
                    RF_STR_PF_ARG(type_str(t->operator.right)));
    case TYPE_CATEGORY_LEAF:
        return RFS_(RF_STR_PF_FMT":"RF_STR_PF_FMT, RF_STR_PF_ARG(t->leaf.id),
                    RF_STR_PF_ARG(type_str(t->leaf.type)));
    default:
        RF_ASSERT(false, "TODO: Not yet implemented");
        break;
    }

    return NULL;
}

/* -- type traversal functions -- */

bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg)
{
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_GENERIC:
        // Do nothing
        break;

    case TYPE_CATEGORY_OPERATOR:
        if (!type_for_each_leaf(t->operator.left, cb, user_arg)) {
            return false;
        }
        if (!type_for_each_leaf(t->operator.right, cb, user_arg)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        if (!cb(&t->leaf, user_arg)) {
            return false;
        }
        break;
    }

    return true;
}
