#include <types/type.h>

#include <Persistent/buffers.h>
#include <Utils/fixed_memory_pool.h>
#include <Utils/bits.h>

#include <module.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <ast/ast.h>
#include <ast/type.h>
#include <ast/generics.h>
#include <ast/vardecl.h>
#include <ast/function.h>
#include <ast/identifier.h>

#include <types/type_elementary.h>
#include <types/type_function.h>


const struct RFstring g_wildcard_s = RF_STRING_STATIC_INIT("_");

/* -- forward declarations of functions -- */
static bool type_operator_init_from_node(struct type_operator *t,
                                         const struct ast_node *n,
                                         struct module *m,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl);

/* -- miscellaneous type functions used internally (static) -- */
struct function_args_ctx {
    struct symbol_table *st;
    struct module *module;
};

// Add all arguments to the symbol table. Leafs are added with key as their
// ID and anonumous types are added with a unique value as their key
static bool do_type_function_add_args_to_st(struct type *t, void *user)
{
    struct function_args_ctx *ctx = user;
    if (t->category == TYPE_CATEGORY_LEAF) {
        return symbol_table_add_type(ctx->st, ctx->module, t->leaf.id, t->leaf.type);
    } else if (type_is_sumop(t)) {
        return symbol_table_add_type(
            ctx->st,
            ctx->module,
            type_get_unique_value_str(t, true),
            t
        );
    }
    return true;
}

static bool type_function_add_args_to_st(struct type *args_t,
                                         struct module *m,
                                         struct symbol_table *st)
{
    struct function_args_ctx ctx;
    ctx.st = st;
    ctx.module = m;

    return type_traverse_postorder(args_t, do_type_function_add_args_to_st, &ctx);
}


/* -- type allocation functions -- */

struct type *type_alloc(struct module *m)
{
    struct type *ret = rf_fixed_memorypool_alloc_element(m->types_pool);
    RF_STRUCT_ZERO(ret);
    return ret;
}

void type_free(struct type *t, struct module *m)
{
    rf_fixed_memorypool_free_element(m->types_pool, t);
}

/* -- type creation and initialization functions used internally -- */

static bool type_leaf_init_from_node(struct type_leaf *leaf,
                                     const struct ast_node *ast_typeleaf,
                                     struct module *m,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl)
{
    struct ast_node *right;
    struct ast_node *left;
    AST_NODE_ASSERT_TYPE(ast_typeleaf, AST_TYPE_LEAF);
    right = ast_typeleaf_right(ast_typeleaf);
    left = ast_typeleaf_left(ast_typeleaf);

    AST_NODE_ASSERT_TYPE(left, AST_IDENTIFIER);
    leaf->id = ast_identifier_str(left);

    if (right->type == AST_XIDENTIFIER) {
        leaf->type = type_lookup_xidentifier(right, m, st, genrdecl);
        if (!leaf->type) {
            RF_ERROR("No type could be looked up for xidentifier \""RF_STR_PF_FMT"\"",
                     RF_STR_PF_ARG(ast_xidentifier_str(right)));
            return false;
        }

    } else if (right->type == AST_TYPE_DESCRIPTION ||
               right->type == AST_TYPE_OPERATOR) {
        leaf->type = module_get_or_create_type(m, right, st, genrdecl);
        if (!leaf->type) {
            return false;
        }

    } else {
        RF_ASSERT_OR_CRITICAL(false, return false,
                              "Illegal ast node type \""RF_STR_PF_FMT"\""
                              " detected as the right part of a type description",
                              RF_STR_PF_ARG(ast_node_str(right)));
    }

    return true;
}

struct type *type_leaf_create_from_node(const struct ast_node *typedesc,
                                        struct module *m,
                                        struct symbol_table *st,
                                        struct ast_node *genrdecl)
{
    struct type *ret;
    ret = type_alloc(m);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    ret->category = TYPE_CATEGORY_LEAF;
    if (!type_leaf_init_from_node(&ret->leaf, typedesc, m, st, genrdecl)) {
        type_free(ret, m);
        return NULL;
    }

    // add it to the types list
    module_types_set_add(m, ret);
    return ret;
}

static bool type_init_from_typeelem(struct type *t,
                                    const struct ast_node *typeelem,
                                    struct module *m,
                                    struct symbol_table *st,
                                    struct ast_node *genrdecl)
{
    switch(typeelem->type) {
    case AST_TYPE_LEAF:
        t->category = TYPE_CATEGORY_LEAF;
        return type_leaf_init_from_node(&t->leaf, typeelem, m, st, genrdecl);
    case AST_TYPE_OPERATOR:
        t->category = TYPE_CATEGORY_OPERATOR;
        return type_operator_init_from_node(&t->operator,
                                            typeelem,
                                            m,
                                            st,
                                            genrdecl);
    case AST_TYPE_DESCRIPTION:
        // case of anonymous type on the right of a typeleaf
        // e.g.:  foo:(i32 | string)
        return type_init_from_typeelem(t, ast_typedesc_desc_get(typeelem), m, st, genrdecl);
    default:
        RF_ASSERT_OR_EXIT(false, "Case should never happen");
        break;
    }
    return false;
}

struct type *type_create_from_typedesc(struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl)
{
    AST_NODE_ASSERT_TYPE(typedesc, AST_TYPE_DESCRIPTION);
    return type_create_from_typeelem(typedesc->typedesc.desc, m, st, genrdecl);
}

struct type *type_create_from_typeelem(const struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl)
{
    struct type *ret;
    ret = type_alloc(m);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    if (!type_init_from_typeelem(ret, typedesc, m, st, genrdecl)) {
        type_free(ret, m);
        return NULL;
    }

    return ret;
}

static bool type_operator_init_from_node(struct type_operator *t,
                                         const struct ast_node *n,
                                         struct module *m,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl)
{
    struct type *left;
    struct type *right;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);

    t->type = ast_typeop_op(n);
    left = type_lookup_or_create(ast_typeop_left(n), m, st, genrdecl, true);
    if (!left) {
        return false;
    }
    right = type_lookup_or_create(ast_typeop_right(n), m, st, genrdecl, true);
    if (!right) {
        return false;
    }

    t->left = left;
    t->right = right;

    return true;
}

struct type *type_lookup_or_create(const struct ast_node *n,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl,
                                   bool make_leaf)
{
    switch (n->type) {
    case AST_XIDENTIFIER:
        return type_lookup_xidentifier(n, m, st, genrdecl);
    case AST_TYPE_LEAF:
        return (make_leaf)
            ? type_leaf_create_from_node(n, m, st, genrdecl)
            : type_lookup_or_create(ast_typeleaf_right(n), m, st, genrdecl, make_leaf);
        break;
    case AST_TYPE_DESCRIPTION:
        return type_lookup_or_create(ast_typedesc_desc_get(n), m, st, genrdecl, make_leaf);
    case AST_TYPE_OPERATOR:
        return module_get_or_create_type(m, n, st, genrdecl);
    case AST_VARIABLE_DECLARATION:
        return type_lookup_or_create(ast_vardecl_desc_get(n), m, st, genrdecl, false);
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Unexpected ast node type "
                              "\""RF_STR_PF_FMT"\" detected",
                              RF_STR_PF_ARG(ast_node_str(n)));
    }

    return NULL;
}

struct type *type_create_from_operation(enum typeop_type type,
                                        struct type *left,
                                        struct type *right,
                                        struct module *m)
{
    struct type *t;
    // TODO: Somehow also check if the type is already existing in the analyzer
    //       and if it is return it instead of creating it
    t = type_alloc(m);
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

struct type *type_create_from_node(const struct ast_node *node,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl)
{
    switch (node->type) {
    case AST_TYPE_DECLARATION:
        return type_create_from_typedecl(node, m, st);
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_OPERATOR:
    case AST_TYPE_LEAF:
        return type_create_from_typeelem(node, m, st, genrdecl);
    case AST_FUNCTION_DECLARATION:
        return type_create_from_fndecl(node, m, st);
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Attempted to create a type "
                              "for illegal ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(node)));
    }

    return NULL;
}

struct type *type_create_from_typedecl(const struct ast_node *n,
                                       struct module *m,
                                       struct symbol_table *st)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_DEFINED;
    t->defined.name = ast_typedecl_name_str(n);
    t->defined.type = type_lookup_or_create(ast_typedecl_typedesc_get(n),
                                            m,
                                            st,
                                            ast_typedecl_genrdecl_get(n),
                                            true);
    if (!t->defined.type) {
        RF_ERROR("Failed to create type for typedecl's typedescription");
        type_free(t, m);
        t = NULL;
    }

    module_types_set_add(m, t);
    return t;
}

static bool type_init_from_fndecl(struct type *t,
                                  const struct ast_node *n,
                                  struct module *m,
                                  struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    struct ast_node *args = ast_fndecl_args_get(n);
    struct ast_node *ret = ast_fndecl_return_get(n);
    struct type *arg_type = NULL;
    struct type *ret_type = NULL;

    // set argument type (left part of the operand)
    if (args) {
        arg_type = type_lookup_or_create(args, m, st,
                                         ast_fndecl_genrdecl_get(n),
                                         true);
        if (!arg_type) {
            return false;
        }
        // also add the function's arguments to its symbol table
        type_function_add_args_to_st(arg_type, m, ast_fndecl_symbol_table_get((struct ast_node*)n));
    } else {
        arg_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
    }

    if (ret) {
        ret_type = type_lookup_or_create(ret, m, st,
                                         ast_fndecl_genrdecl_get(n),
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

struct type *type_create_from_fndecl(const struct ast_node *n,
                                     struct module *m,
                                     struct symbol_table *st)
{
    struct type *t;
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);

    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    if (!type_init_from_fndecl(t, n, m, st)) {
        type_free(t, m);
        t = NULL;
    }

    module_types_set_add(m, t);
    return t;
}

struct type *type_function_create(struct module *m,
                                  struct type *arg_type,
                                  struct type *ret_type)
{
    struct type *t;
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    type_function_init(t, arg_type, ret_type);
    return t;
}

struct type *type_module_create(struct module *m, const struct RFstring *name)
{
    struct type *t;
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->module.name = name;
    return t;
}

struct type *type_leaf_create(struct module *m,
                              const struct RFstring *id,
                              struct type *leaf_type)
{
    struct type *t;
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_LEAF;
    t->leaf.id = id;
    t->leaf.type = leaf_type;
    module_types_set_add(m, t);
    return t;
}

struct type *type_operator_create(struct module *m,
                                  struct type *left_type,
                                  struct type *right_type,
                                  enum typeop_type type)
{
    struct type *t;
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = type;
    t->operator.left = left_type;
    t->operator.right = right_type;
    module_types_set_add(m, t);
    return t;
}

struct type *type_operator_create_from_node(struct ast_node *n,
                                            struct module *m,
                                            struct symbol_table *st,
                                            struct ast_node *genrdecl)
{
    struct type *t;
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    t->category = TYPE_CATEGORY_OPERATOR;
    if (!type_operator_init_from_node(&t->operator, n, m, st, genrdecl)) {
        type_free(t, m);
        t = NULL;
    }

    module_types_set_add(m, t);
    return t;
}

/* -- type getters -- */
struct type *type_lookup_xidentifier(const struct ast_node *n,
                                     struct module *mod,
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
    analyzer_err(mod, ast_node_startmark(n),
                 ast_node_endmark(n),
                 "Type \""RF_STR_PF_FMT"\" is not defined",
                 RF_STR_PF_ARG(id));
    return NULL;

}
struct type *type_lookup_identifier_string(const struct RFstring *str,
                                           const struct symbol_table *st)
{
    struct symbol_table_record *rec;
    int elementary_type;

    if (string_is_wildcard(str)) {
        return (struct type*)type_get_wildcard();
    }

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

static struct RFstring *type_str_do(const struct type *t, int options)
{

    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        return (struct RFstring*)type_elementary_get_str(t->elementary.etype);
    case TYPE_CATEGORY_OPERATOR:
    {
        struct RFstring *sleft;
        struct RFstring *sright;
        sleft = type_str_do(t->operator.left, options);
        sright = type_str_do(t->operator.right, options);
        if (!sleft || !sright) {
            return NULL;
        }

        return RFS(RF_STR_PF_FMT RF_STR_PF_FMT RF_STR_PF_FMT,
                   RF_STR_PF_ARG(sleft),
                   RF_STR_PF_ARG(type_op_str(t->operator.type)),
                   RF_STR_PF_ARG(sright));
    }
    case TYPE_CATEGORY_LEAF:
    {
        struct RFstring *sleaf_type;
        sleaf_type = type_str_do(t->leaf.type, options);
        if (!sleaf_type) {
            return NULL;
        }
        return RF_BITFLAG_ON(options, TSTR_LEAF_ID)
                ? RFS(RF_STR_PF_FMT":"RF_STR_PF_FMT,
                      RF_STR_PF_ARG(t->leaf.id),
                      RF_STR_PF_ARG(sleaf_type))
                : RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(sleaf_type));
    }
    case TYPE_CATEGORY_DEFINED:
        return RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(t->defined.name));
    case TYPE_CATEGORY_WILDCARD:
        return RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(&g_wildcard_s));
    default:
        RF_ASSERT(false, "TODO: Not yet implemented");
        break;
    }

    return NULL;
}

struct RFstring *type_str(const struct type *t, int options)
{
    if (t->category == TYPE_CATEGORY_DEFINED &&
        RF_BITFLAG_ON(options, TSTR_DEFINED_CONTENTS)) {

        RF_BITFLAG_UNSET(options, TSTR_DEFINED_CONTENTS);
        struct RFstring *sdefined = type_str_do(t->defined.type, options);
        if (!sdefined) {
            return NULL;
        }
        return RFS(RF_STR_PF_FMT" {" RF_STR_PF_FMT "}",
                   RF_STR_PF_ARG(t->defined.name),
                   RF_STR_PF_ARG(sdefined));
    } else {
        return type_str_do(t, options);
    }
}
i_INLINE_INS const struct rir_type *type_get_rir_or_die(const struct type *type);
i_INLINE_INS struct RFstring *type_str_or_die(const struct type *t, int options);

size_t type_get_uid(const struct type *t, bool count_leaf_id)
{
    size_t ret;
    struct RFstring *str;
    RFS_PUSH();
    str = count_leaf_id ? type_str_or_die(t, TSTR_LEAF_ID | TSTR_DEFINED_CONTENTS)
        : type_str_or_die(t, TSTR_DEFINED_CONTENTS);
    ret = rf_hash_str_stable(str, 0);
    RFS_POP();
    return ret;
}



const struct RFstring *type_get_unique_value_str(const struct type *t, bool count_leaf_id)
{
    return t->category == TYPE_CATEGORY_DEFINED
        ? RFS_OR_DIE("internal_struct_val_%u", type_get_uid(t->defined.type, count_leaf_id))
        : RFS_OR_DIE("internal_struct_val_%u", type_get_uid(t, count_leaf_id));
}

const struct RFstring *type_get_unique_type_str(const struct type *t, bool count_leaf_id)
{
    if (t->category == TYPE_CATEGORY_DEFINED) {
        t = t->defined.type;
    }
    return RFS_OR_DIE("internal_struct_%u", type_get_uid(t, count_leaf_id));
}

static struct type g_wildcard_type = {.category=TYPE_CATEGORY_WILDCARD};
const struct type *type_get_wildcard()
{
    return &g_wildcard_type;
}

const struct RFstring *type_defined_get_name(const struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_DEFINED, "Called with non defined type category");
    return t->defined.name;
}

i_INLINE_INS bool type_is_sumop(const struct type *t);
i_INLINE_INS bool type_is_sumtype(const struct type *t);
i_INLINE_INS const struct type *type_get_nth_type_or_die(const struct type *t, unsigned int index);
i_INLINE_INS const struct RFstring *type_get_nth_name_or_die(const struct type *t, unsigned int index);

/* -- type traversal functions -- */

bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg)
{
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
    case TYPE_CATEGORY_GENERIC:
    case TYPE_CATEGORY_WILDCARD:
    case TYPE_CATEGORY_FOREIGN_FUNCTION:
    case TYPE_CATEGORY_MODULE:
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
    case TYPE_CATEGORY_WILDCARD:
    case TYPE_CATEGORY_FOREIGN_FUNCTION:
    case TYPE_CATEGORY_MODULE:
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
    case TYPE_CATEGORY_MODULE:
    case TYPE_CATEGORY_FOREIGN_FUNCTION:
    case TYPE_CATEGORY_WILDCARD:
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
    default:
        RF_ASSERT(false, "Not implemented type category for postorder iteration");
        return false;
    }

    return post_cb(t, user_arg);
}
