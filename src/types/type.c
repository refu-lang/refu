#include <types/type.h>

#include <Persistent/buffers.h>

#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <ast/ast.h>
#include <ast/type.h>
#include <ast/generics.h>
#include <ast/vardecl.h>
#include <ast/function.h>

#include <types/type_builtin.h>

/* -- forward declarations of functions -- */
static bool type_composite_init(struct type_composite *t, struct ast_node *n,
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
static bool type_function_add_args_to_st(struct type_function *ft,
                                         struct analyzer *a,
                                         struct symbol_table *st)
{
    struct function_args_ctx ctx;
    ctx.st = st;
    ctx.analyzer = a;
    return type_for_each_leaf(ft->argument_type,
                              do_type_function_add_args_to_st, &ctx);
}


/* -- type allocation functions -- */

struct type *type_alloc(struct analyzer *a)
{
    return rf_fixed_memorypool_alloc_element(&a->types_pool);
}

void type_free(struct type *t, struct analyzer *a)
{
    rf_fixed_memorypool_free_element(&a->types_pool, t);
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
        leaf->type = analyzer_get_or_create_anonymous_type(a, right, st, genrdecl);
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
        t->category = TYPE_CATEGORY_LEAF;
        return type_leaf_init(&t->leaf, typedesc, a, st, genrdecl);
    } else {
        t->category = TYPE_CATEGORY_ANONYMOUS;
        return type_composite_init(&t->anonymous, typedesc, a, st, genrdecl);
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

static bool type_composite_init(struct type_composite *t, struct ast_node *n,
                                struct analyzer *a, struct symbol_table *st,
                                struct ast_node *genrdecl)
{
    if (n->type == AST_TYPE_OPERATOR) {
        struct type *left;
        struct type *right;

        t->is_operator = true;
        t->op.type = ast_typeop_op(n);

        left = type_create(ast_typeop_left(n), a, st, genrdecl);
        if (!left) {
            return false;
        }
        right = type_create(ast_typeop_right(n), a, st, genrdecl);
        if (!right) {
            return false;
        }

        t->op.left = left;
        t->op.right = right;


    } else if (n->type == AST_TYPE_DESCRIPTION) {
        t->is_operator = false;
        if (!type_leaf_init(&t->leaf, n, a, st, genrdecl)) {
            return false;
        }

    } else {
        RF_ASSERT_OR_CRITICAL(false,"Illegal ast node type "
                              "\""RF_STR_PF_FMT"\" detected in typedesec",
                              RF_STR_PF_ARG(ast_node_str(n)));
        return false;
    }

    return true;
}



static struct type *type_composite_lookup_or_create(struct ast_node *n,
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
        return analyzer_get_or_create_anonymous_type(a, n, st, genrdecl);
    default:
        RF_ASSERT_OR_CRITICAL(false,"Unexpected ast node type "
                              "\""RF_STR_PF_FMT"\" detected",
                              RF_STR_PF_ARG(ast_node_str(n)));
    break;

    }

    return NULL;
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
    struct type *t;
    struct type *temp;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);

    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    t->category = TYPE_CATEGORY_USER_DEFINED;
    t->defined.id = ast_typedecl_name_str(n);
    temp = type_composite_lookup_or_create(ast_typedecl_typedesc_get(n), a, st,
                                           ast_typedecl_genrdecl_get(n));
    if (!temp) {
        return NULL;
    }
    t->defined.type = &temp->anonymous;

    return t;
}

struct type *type_create_from_vardecl(struct ast_node *n,
                                      struct analyzer *a,
                                      struct symbol_table *st)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(n, AST_VARIABLE_DECLARATION);

    t = type_composite_lookup_or_create(ast_vardecl_desc_get(n), a, st, NULL);
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

    t->category = TYPE_CATEGORY_FUNCTION;

    t->function.argument_type = NULL;
    if (args) {
        t->function.argument_type = type_composite_lookup_or_create(
            args, a, st, ast_fndecl_genrdecl_get(n));
        if (!t->function.argument_type) {
            // TODO: Free argument_type if created
            return false;
        }

        // also add the function's arguments to its symbol table
        type_function_add_args_to_st(&t->function, a, ast_fndecl_symbol_table_get(n));
    }

    t->function.return_type = NULL;
    if (ret) {
        t->function.return_type = type_composite_lookup_or_create(
            ret, a, st, ast_fndecl_genrdecl_get(n));

        if (!t->function.return_type) {
            // TODO: Free return_type if created
            return false;
        }
    }

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

struct type *type_anonymous_create(struct ast_node *n,
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

    t->category = TYPE_CATEGORY_ANONYMOUS;
    if (!type_composite_init(&t->anonymous, n, a, st, genrdecl)) {
        type_free(t, a);
        t = NULL;
    }

    return t;
}

/* -- type comparison functions -- */

i_INLINE_INS void type_comparison_ctx_init(struct type_comparison_ctx *ctx,
                                           enum comparison_reason reason);

static inline bool type_category_equals(const struct type *t1,
                                        const struct type *t2)
{
    if (t1->category == t2->category) {
        return true;
    }

    if (t1->category == TYPE_CATEGORY_LEAF &&
        t2->category == TYPE_CATEGORY_ANONYMOUS && (!t2->anonymous.is_operator)) {
        return true;
    }

    if (t2->category == TYPE_CATEGORY_LEAF &&
        t1->category == TYPE_CATEGORY_ANONYMOUS && (!t1->anonymous.is_operator)) {
        return true;
    }

    return false;
}

static inline bool type_function_equals(const struct type_function *t1,
                                        const struct type_function *t2,
                                        struct type_comparison_ctx *ctx)
{
    return type_equals(t1->argument_type, t2->argument_type, ctx) &&
        type_equals( t1->return_type, t2->return_type, ctx);
}

static inline bool type_leaf_equals(const struct type_leaf *t1,
                                    const struct type_leaf *t2,
                                    struct type_comparison_ctx *ctx)
{
    return type_equals(t1->type, t2->type, ctx);
}

static bool type_composite_equals(const struct type_composite *t1,
                                  const struct type_composite *t2,
                                  struct type_comparison_ctx *ctx)
{
    if (t1 == t2) {
        return true;
    }

    if (t1->is_operator != t2->is_operator) {
        return false;
    }

    if (t1->is_operator) {
        if (t1->op.type != t2->op.type) {
            return false;
        }

        return type_equals(t1->op.left, t2->op.left, ctx) &&
            type_equals(t1->op.right, t2->op.right, ctx);
    }

    return type_leaf_equals(&t1->leaf, &t2->leaf, ctx);

}

bool type_equals(const struct type* t1, const struct type *t2,
                 struct type_comparison_ctx *ctx)
{
    // first check if we refer to the same type (builtin or anonymous)
    if (t1 == t2) {
        return true;
    }

    if (!type_category_equals(t1, t2)) {
        return false;
    }

    switch (t1->category) {
    case TYPE_CATEGORY_USER_DEFINED:
        return type_composite_equals(t1->defined.type, t2->defined.type, ctx);
    case TYPE_CATEGORY_ANONYMOUS:
        if (t2->category == TYPE_CATEGORY_LEAF) {
            //handle special case where we compare composite to a leaf composite type
            return type_leaf_equals(&t1->anonymous.leaf, &t2->leaf, ctx);
        } else {
            return type_composite_equals(&t1->anonymous, &t2->anonymous, ctx);
        }
    case TYPE_CATEGORY_BUILTIN:
        return type_builtin_equals(&t1->builtin, &t2->builtin, ctx);
    case TYPE_CATEGORY_LEAF:
        if (t2->category == TYPE_CATEGORY_ANONYMOUS) {
            //handle special case where we compare leaf to a composite leaf type
            return type_leaf_equals(&t1->leaf, &t2->anonymous.leaf, ctx);
        } else {
            return type_leaf_equals(&t1->leaf, &t2->leaf, ctx);
        }
    case TYPE_CATEGORY_FUNCTION:
        return type_function_equals(&t1->function, &t2->function, ctx);
    case TYPE_CATEGORY_GENERIC:
        //TODO
        break;
    }

    return false;
}

bool type_equals_typedesc(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl)
{
    struct type *looked_up_t;
    if (type_desc->type == AST_TYPE_OPERATOR) {
        struct type_composite *composite;

        if (t->category == TYPE_CATEGORY_USER_DEFINED) {
            composite = t->defined.type;
        } else if (t->category != TYPE_CATEGORY_ANONYMOUS) {
            composite = &t->anonymous;
        } else {
            return false;
        }
        if (!composite->is_operator) {
            return false;
        }

        if (composite->op.type != ast_typeop_op(type_desc)) {
            return false;
        }

        return type_equals_typedesc(composite->op.left,
                                    ast_typeop_left(type_desc),
                                    a, st, genrdecl) &&
            type_equals_typedesc(composite->op.right,
                                 ast_typeop_right(type_desc),
                                 a, st, genrdecl);

    } else if (type_desc->type == AST_TYPE_DESCRIPTION) {
        AST_NODE_ASSERT_TYPE(ast_typedesc_left(type_desc), AST_IDENTIFIER);
        return type_equals_typedesc(t, ast_typedesc_right(type_desc),
                                    a, st, genrdecl);

    } else if (type_desc->type == AST_XIDENTIFIER) {
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
    struct symbol_table_record *rec;
    bool at_first_st;
    int builtin_type;
    const struct RFstring *id;

    AST_NODE_ASSERT_TYPE(n, AST_XIDENTIFIER);
    id = ast_xidentifier_str(n);


    // check if it's a builtin type
    builtin_type = type_builtin_identifier_p(id);
    if (builtin_type != -1) {
        return (struct type*)type_builtin_get_type(builtin_type);
    }

    // if not check if we know about it from the symbol tables
    rec = symbol_table_lookup_record(st, id, &at_first_st);
    if (rec) {
        return symbol_table_record_type(rec);
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

const struct RFstring *type_str(const struct type *t, struct RFbuffer *buff)
{
    const struct RFstring *ret_str;
    switch(t->category) {
    case TYPE_CATEGORY_BUILTIN:
        ret_str = type_builtin_get_str(t->builtin.btype);
        if (buff) {
            // This is just to show myself how to do it, no need to do it for
            // a builtin type
            return rf_persistent_buffer_create_str_from_str(buff, ret_str);
        }
        return ret_str;
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
    case TYPE_CATEGORY_BUILTIN:
    case TYPE_CATEGORY_GENERIC:
        // Do nothing
        break;
    case TYPE_CATEGORY_ANONYMOUS:
        if (t->anonymous.is_operator) {
            if (!type_for_each_leaf(t->anonymous.op.left, cb, user_arg)) {
                return false;
            }
            if (!type_for_each_leaf(t->anonymous.op.right, cb, user_arg)) {
                return false;
            }
        } else {
            if (!cb(&t->anonymous.leaf, user_arg)) {
                return false;
            }
        }

        break;
    case TYPE_CATEGORY_USER_DEFINED:
        RF_ASSERT(false, "Should not get here");
        return false;
    case TYPE_CATEGORY_FUNCTION:
        if (!type_for_each_leaf(t->function.argument_type, cb, user_arg)) {
            return false;
        }
        if (!type_for_each_leaf(t->function.return_type, cb, user_arg)) {
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
