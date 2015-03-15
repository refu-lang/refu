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
                               struct ast_node *genrdecl, bool add_type_to_list);

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

static bool type_leaf_init_from_typedesc(struct type_leaf *leaf,
                                         struct ast_node *typedesc,
                                         struct analyzer *a,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl,
                                         bool add_type_to_list)
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
        leaf->type = analyzer_get_or_create_type(a, right, st, genrdecl, add_type_to_list);
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

static struct type *type_leaf_create_from_typedesc(struct ast_node *typedesc,
                                                   struct analyzer *a,
                                                   struct symbol_table *st,
                                                   struct ast_node *genrdecl,
                                                   bool add_type_to_list)
{
    struct type *ret;
    ret = type_alloc(a);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    ret->category = TYPE_CATEGORY_LEAF;
    if (!type_leaf_init_from_typedesc(&ret->leaf, typedesc, a, st, genrdecl, add_type_to_list)) {
        type_free(ret, a);
        return NULL;
    }

    // add it to the types list
    rf_ilist_add(&a->composite_types, &ret->lh);
    return ret;
}

static bool type_init_from_typedesc(struct type *t, struct ast_node *typedesc,
                                    struct analyzer *a, struct symbol_table *st,
                                    struct ast_node *genrdecl,
                                    bool add_type_to_list)
{
    if (ast_types_left(typedesc)->type == AST_IDENTIFIER) {
        AST_NODE_ASSERT_TYPE(typedesc, AST_TYPE_DESCRIPTION);

        t->category = TYPE_CATEGORY_LEAF;
        return type_leaf_init_from_typedesc(&t->leaf, typedesc, a, st, genrdecl, add_type_to_list);
    } else {
        t->category = TYPE_CATEGORY_OPERATOR;
        return type_operator_init(&t->operator, typedesc, a, st, genrdecl, add_type_to_list);
    }
}

struct type *type_create_from_typedesc(struct ast_node *typedesc,
                                       struct analyzer *a,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl,
                                       bool add_type_to_list)
{
    struct type *ret;
    ret = type_alloc(a);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    if (!type_init_from_typedesc(ret, typedesc, a, st, genrdecl, add_type_to_list)) {
        type_free(ret, a);
        return NULL;
    }

    return ret;
}

static bool type_operator_init(struct type_operator *t, struct ast_node *n,
                               struct analyzer *a, struct symbol_table *st,
                               struct ast_node *genrdecl,
                               bool add_type_to_list)
{
    struct type *left;
    struct type *right;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);

    t->type = ast_typeop_op(n);
    left = type_lookup_or_create(ast_typeop_left(n), a, st, genrdecl, true, add_type_to_list);
    if (!left) {
        return false;
    }
    right = type_lookup_or_create(ast_typeop_right(n), a, st, genrdecl, true, add_type_to_list);
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
                                   struct ast_node *genrdecl,
                                   bool make_leaf,
                                   bool add_type_to_list)
{
    switch (n->type) {
    case AST_XIDENTIFIER:
        // lookup the xidentifier itself
        return type_lookup_xidentifier(n, a, st, genrdecl);
    case AST_TYPE_DESCRIPTION:
        // if it's a simple type description (e.g. only identifier COLON xidentifier
        if (ast_typedesc_left(n)->type == AST_IDENTIFIER &&
            ast_typedesc_right(n)->type == AST_XIDENTIFIER) {

            if (make_leaf) {
                return type_leaf_create_from_typedesc(n, a, st, genrdecl, add_type_to_list);
            } else {
                return type_lookup_xidentifier(ast_typedesc_right(n), a, st, genrdecl);
            }
        }
        // else fall through case, same as type operator
    case AST_TYPE_OPERATOR:
        return analyzer_get_or_create_type(a, n, st, genrdecl, add_type_to_list);
    case AST_VARIABLE_DECLARATION:
        return type_lookup_or_create(ast_vardecl_desc_get(n), a, st, genrdecl, false, add_type_to_list);
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
    //       and if it is return it instead of creating it
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
                         struct ast_node *genrdecl,
                         bool add_type_to_list)
{
    switch (node->type) {
    case AST_TYPE_DECLARATION:
        return type_create_from_typedecl(node, a, st);
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_OPERATOR:
        return type_create_from_typedesc(node, a, st, genrdecl, add_type_to_list);
    case AST_FUNCTION_DECLARATION:
        return type_create_from_fndecl(node, a, st);
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
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_DEFINED;
    t->defined.name = ast_typedecl_name_str(n);
    t->defined.type = type_lookup_or_create(ast_typedecl_typedesc_get(n),
                                            a,
                                            st,
                                            ast_typedecl_genrdecl_get(n),
                                            true,
                                            true);
    if (!t->defined.type) {
        RF_ERROR("Failed to create type for typedecl's typedescription");
        type_free(t, a);
        t = NULL;
    }
    // add it to the types list
    rf_ilist_add(&a->composite_types, &t->lh);
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
                                         ast_fndecl_genrdecl_get(n),
                                         true,
                                         true);
        if (!arg_type) {
            return false;
        }
        // also add the function's arguments to its symbol table
        type_function_add_args_to_st(arg_type, a, ast_fndecl_symbol_table_get(n));
    } else {
        arg_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
    }

    if (ret) {
        ret_type = type_lookup_or_create(ret, a, st,
                                         ast_fndecl_genrdecl_get(n),
                                         true,
                                         true);
        if (!ret_type) {
            return false;
        }
    } else {
        ret_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
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

    // add it to the types list
    rf_ilist_add(&a->composite_types, &t->lh);
    return t;
}

struct type *type_function_create(struct analyzer *a,
                                  struct type *arg_type,
                                  struct type *ret_type)
{
    struct type *t;
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    type_function_init(t, arg_type, ret_type);
    return t;
}

struct type *type_leaf_create(struct analyzer *a,
                              const struct RFstring *id,
                              struct type *leaf_type)
{
    struct type *t;
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_LEAF;
    t->leaf.id = id;
    t->leaf.type = leaf_type;
    return t;
}


struct type *type_operator_create(struct ast_node *n,
                                  struct analyzer *a,
                                  struct symbol_table *st,
                                  struct ast_node *genrdecl,
                                  bool add_type_to_list)
{
    struct type *t;
    t = type_alloc(a);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    t->category = TYPE_CATEGORY_OPERATOR;
    if (!type_operator_init(&t->operator, n, a, st, genrdecl, add_type_to_list)) {
        type_free(t, a);
        t = NULL;
    }

    // add it to the types list
    rf_ilist_add(&a->composite_types, &t->lh);
    return t;
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
    int elementary_type;

    // check if it's an elementary type
    elementary_type = type_elementary_identifier_p(str);
    if (elementary_type != -1) {
        return (struct type*)type_elementary_get_type(elementary_type);
    }

    // if not check if we know about it from the symbol tables
    rec = symbol_table_lookup_record(st, str, NULL);
    if (rec) {
        return symbol_table_record_type(rec);
    }

    return NULL;
}

const struct RFstring *type_defined_to_str(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_DEFINED, "Non user defined type provided");
    return RFS_("type "RF_STR_PF_FMT"{ " RF_STR_PF_FMT "}",
                RF_STR_PF_ARG(t->defined.name),
                RF_STR_PF_ARG(type_str(t->defined.type, true)));
}

const struct RFstring *type_str(const struct type *t, bool print_leaf_id)
{
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        return type_elementary_get_str(t->elementary.etype);
    case TYPE_CATEGORY_OPERATOR:
        return RFS_(RF_STR_PF_FMT RF_STR_PF_FMT RF_STR_PF_FMT,
                    RF_STR_PF_ARG(type_str(t->operator.left, print_leaf_id)),
                    RF_STR_PF_ARG(type_op_str(t->operator.type)),
                    RF_STR_PF_ARG(type_str(t->operator.right, print_leaf_id)));
    case TYPE_CATEGORY_LEAF:
        if (print_leaf_id) {
        return RFS_(RF_STR_PF_FMT":"RF_STR_PF_FMT, RF_STR_PF_ARG(t->leaf.id),
                    RF_STR_PF_ARG(type_str(t->leaf.type, print_leaf_id)));
        } else {
            return RFS_(RF_STR_PF_FMT, RF_STR_PF_ARG(type_str(t->leaf.type, print_leaf_id)));
        }
    case TYPE_CATEGORY_DEFINED:
        return RFS_(RF_STR_PF_FMT, RF_STR_PF_ARG(t->defined.name));

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

    case TYPE_CATEGORY_DEFINED:
        if (!type_for_each_leaf(t->defined.type, cb, user_arg)) {
            return false;
        }
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

enum traversal_cb_res type_for_each_leaf_nostop(const struct type *t, leaf_type_nostop_cb cb, void *user_arg)
{
    enum traversal_cb_res rc = TRAVERSAL_CB_OK;
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_GENERIC:
        // Do nothing
        break;

    case TYPE_CATEGORY_DEFINED:
        rc = type_for_each_leaf_nostop(t->defined.type, cb, user_arg);
        if (traversal_stop(rc)) {
            return rc;
        }
        break;

    case TYPE_CATEGORY_OPERATOR:
        rc = type_for_each_leaf_nostop(t->operator.left, cb, user_arg);
        if (traversal_stop(rc)) {
            return rc;
        }
        rc = type_for_each_leaf_nostop(t->operator.right, cb, user_arg);
        if (traversal_stop(rc)) {
            return rc;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        return cb(&t->leaf, user_arg);
    }

    return rc;
}

bool type_traverse_postorder(struct type *t, type_iterate_cb cb, void *user_arg)
{
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_LEAF:
        break;
    case TYPE_CATEGORY_DEFINED:
        if (!type_traverse_postorder(t->defined.type, cb, user_arg)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_OPERATOR:
        if (!type_traverse_postorder(t->operator.left, cb, user_arg)) {
            return false;
        }
        if (!type_traverse_postorder(t->operator.right, cb, user_arg)) {
            return false;
        }
        break;
    default:
        RF_ASSERT(false, "Not implemented type category for postorder iteration");
        return false;
    }

    return cb(t, user_arg);
}

bool type_traverse(struct type *t, type_iterate_cb pre_cb,
                   type_iterate_cb post_cb, void *user_arg)
{
    if (!pre_cb(t, user_arg)) {
        return false;
    }
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_LEAF:
        break;
    case TYPE_CATEGORY_DEFINED:
        if (!type_traverse(t->defined.type, pre_cb, post_cb, user_arg)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_OPERATOR:
        if (!type_traverse(t->operator.left, pre_cb, post_cb, user_arg)) {
            return false;
        }
        if (!post_cb(t, user_arg)) {
            return false;
        }
        return type_traverse(t->operator.right, pre_cb, post_cb, user_arg);
        break;
    default:
        RF_ASSERT(false, "Not implemented type category for postorder iteration");
        return false;
    }

    return post_cb(t, user_arg);    
}
