#include <types/type.h>
#include <types/type_arr.h>
#include <types/type_operators.h>
#include <types/type_function.h>
#include <types/type_comparisons.h>

#include <rfbase/utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <ast/vardecl.h>
#include <ast/type.h>
#include <ast/function.h>
#include <ast/generics.h>
#include <analyzer/type_set.h>
#include <analyzer/analyzer.h>
#include <module.h>

struct type_creation_ctx {
    //! A queue of type operators during creation
    struct {darray(struct type_operator*);} operators;

    /* -- arguments to be passed down to type creation functions -- */
    struct module *m;
    struct symbol_table *st;
    struct ast_node *genrdecl;
    struct type_arr *arr;
};
i_THREAD__ struct type_creation_ctx *g_type_creation_ctx = NULL;


void type_creation_ctx_init()
{
    RF_ASSERT_OR_EXIT(!g_type_creation_ctx, "Global type creation context was already initialized.");
    g_type_creation_ctx = calloc(sizeof(struct type_creation_ctx), 1);
    RF_ASSERT_OR_EXIT(g_type_creation_ctx, "Could not allocate a global type creation context");
    darray_init(g_type_creation_ctx->operators);
}

void type_creation_ctx_deinit()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    darray_clear(g_type_creation_ctx->operators);
    darray_free(g_type_creation_ctx->operators);
    free(g_type_creation_ctx);
    g_type_creation_ctx = NULL;
}

static inline void type_creation_ctx_push_op(struct type_operator *op)
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    darray_append(g_type_creation_ctx->operators, op);
}

static inline void type_creation_ctx_pop_op()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    RF_ASSERT(darray_size(g_type_creation_ctx->operators) != 0, "Tried to pop non-existing typeop");
    (void)darray_pop(g_type_creation_ctx->operators);
}

void type_creation_ctx_set_args(
    struct module *m,
    struct symbol_table *st,
    struct ast_node *genrdecl
)
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    g_type_creation_ctx->m = m;
    g_type_creation_ctx->st = st;
    g_type_creation_ctx->genrdecl = genrdecl;
}

static inline struct module *type_creation_ctx_mod()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    return g_type_creation_ctx->m;
}

static inline struct symbol_table *type_creation_ctx_st()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    return g_type_creation_ctx->st;
}

static inline struct ast_node *type_creation_ctx_genrdecl()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    return g_type_creation_ctx->genrdecl;
}

static inline struct type_arr *type_creation_ctx_arr()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    return g_type_creation_ctx->arr;
}

static inline void type_creation_ctx_set_genrdecl(struct ast_node *n)
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    g_type_creation_ctx->genrdecl = n;
}

static struct type_operator *type_creation_ctx_top_op()
{
    RF_ASSERT(g_type_creation_ctx, "No global type creation context exists");
    return darray_size(g_type_creation_ctx->operators) == 0
        ? NULL
        : darray_top(g_type_creation_ctx->operators);
}

bool type_add_to_currop(struct type* t)
{
    struct type_operator *currop = type_creation_ctx_top_op();
    if (currop) {
        if (type_is_operator(t)) {
            if (currop != &t->operator) { // only if not the same exact operator
                if (type_typeop_get(t) == currop->type) {
                    // if it's the same type operator just append operands to the current one
                    struct type **operand;
                    darray_foreach(operand, t->operator.operands) {
                        darray_append(currop->operands, *operand);
                    }
                } else { // add the the whole operator as a child
                    darray_append(currop->operands, t);
                }
            }
        } else {
            darray_append(currop->operands, t);
        }
    }
    return true;
}

/* -- forward declarations of functions -- */
static bool type_operator_init_from_node(struct type *t, const struct ast_node *n);

/* -- miscellaneous type functions used internally (static) -- */
struct function_args_ctx {
    struct symbol_table *function_st;
    struct symbol_table *parent_st;
    struct module *module;
    bool success;
};

static inline void function_args_ctx_init(
    struct function_args_ctx *ctx,
    struct symbol_table *function_st,
    struct symbol_table *parent_st,
    struct module *m
)
{
    ctx->function_st = function_st;
    ctx->parent_st = parent_st;
    ctx->module = m;
    ctx->success = true;
}

// Add all arguments to the symbol table. Leafs are added with key as their
// ID and anonymous types are added with a unique value as their key
static bool type_function_add_args_to_st(
    const struct RFstring *name,
    const struct ast_node *ast_desc,
    struct type *t,
    struct function_args_ctx *ctx
)
{
    // lookup ast_desc (which is the right part of the symbol table and set that as the ast node
    AST_NODE_ASSERT_TYPE(ast_desc, AST_XIDENTIFIER);
    const struct RFstring *typename = ast_identifier_str(ast_desc);
    if (type_elementary_identifier_p(typename) == -1) {
        struct symbol_table_record *rec = symbol_table_lookup_record(
            ctx->parent_st,
            typename,
            NULL
        );
        if (!rec) {
            RF_ERROR("At this point the identifier should have been found in the symbol table");
            ctx->success = false;
            return false;
        }

        if (type_is_defined(rec->data)) {
            if (!rec->node) {
                RF_ERROR("At this point the record should have an associated node");
                ctx->success = false;
                return false;
            }
            ast_desc = rec->node;
        }
    }
    ctx->success = symbol_table_add_type(
        ctx->function_st, ctx->module, name, t, ast_desc
    );
    return ctx->success;
}

/* -- type allocation functions -- */

struct type *type_alloc(struct module *m)
{
    struct type *ret = rf_fixed_memorypool_alloc_element(m->types_pool);
    RF_STRUCT_ZERO(ret);
    return ret;
}

struct type *type_alloc_copy(struct module *m, const struct type *source)
{
    struct type *ret = rf_fixed_memorypool_alloc_element(m->types_pool);
    memcpy(ret, source, sizeof(*source));
    return ret;
}

void type_free(struct type *t, struct rf_fixed_memorypool *pool)
{
    RF_ASSERT(pool, "Can't free type without a memory pool");
    if (t->category == TYPE_CATEGORY_OPERATOR) {
        darray_free(t->operator.operands);
    }
    if (t->category == TYPE_CATEGORY_ARRAY) {
        type_array_destroy(t);
    }
    rf_fixed_memorypool_free_element(pool, t);
}

/* -- type creation and initialization functions used internally -- */
static bool type_init_from_typeelem(struct type *t, const struct ast_node *elem)
{
    switch(elem->type) {
    case AST_TYPE_OPERATOR:
        return type_operator_init_from_node(t, elem);
    case AST_TYPE_DESCRIPTION:
        // case of anonymous type on the right of a typeleaf
        // e.g.:  foo:(i32 | string)
        return type_init_from_typeelem(t, ast_typedesc_desc_get(elem));
    default:
        RF_CRITICAL_FAIL("Case should never happen");
        break;
    }
    return false;
}

static struct type *type_operator_lookup_or_create(const struct ast_node *opdesc)
{
    struct type_operator *currop = type_creation_ctx_top_op();
    if (currop && currop->type == ast_typeop_op(opdesc)) {
        // if we are already inside a type operator of the same type
        struct type *left = type_lookup_or_create(ast_typeop_left(opdesc));
        if (!left) {
            RF_ERROR("Failed to find the left type of a typeop");
            return NULL;
        }
        struct type *right = type_lookup_or_create(ast_typeop_right(opdesc));
        if (!right) {
            RF_ERROR("Failed to find the right type of a typeop");
            return NULL;
        }
        return typeop_to_type(currop);
    } else {
        return module_get_or_create_type(opdesc);
    }
}

struct type *type_create_from_typeelem(const struct ast_node *typedesc)
{
    struct type *ret;
    struct module *m = type_creation_ctx_mod();
    RF_ASSERT(typedesc->type != AST_TYPE_LEAF && typedesc->type != AST_XIDENTIFIER,
              "Typeleaf or identifier should never get here");

    ret = type_alloc(m);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    if (!type_init_from_typeelem(ret, typedesc)) {
        type_free(ret, m->types_pool);
        return NULL;
    }

    return ret;
}

struct type *type_lookup_or_create(const struct ast_node *n)
{
    struct type *ret = NULL;
    switch (n->type) {
    case AST_XIDENTIFIER:
        ret = type_lookup_xidentifier(n);
        break;
    case AST_TYPE_LEAF:
        ret = module_get_or_create_type(ast_typeleaf_right(n));
        break;
    case AST_TYPE_DESCRIPTION:
        return type_lookup_or_create(ast_typedesc_desc_get(n));
    case AST_TYPE_OPERATOR:
        ret = type_operator_lookup_or_create(n);
        break;
    case AST_VARIABLE_DECLARATION:
        return type_lookup_or_create(ast_vardecl_desc_get(n));
    default:
        RF_ASSERT_OR_CRITICAL(
            false, return false,
            "Unexpected ast node type \""RFS_PF"\" detected",
            RFS_PA(ast_node_str(n))
        );
        break;
    }

    // we should get here only in the case of end type so add to current operator if existing
    if (ret) {
        type_add_to_currop(ret);
    }
    return ret;
}

struct type *type_create_from_operation(
    enum typeop_type typeop,
    const struct ast_node *n,
    struct type *left,
    struct type *right,
    struct module *m)
{
    struct type *t;
    if (left->category == TYPE_CATEGORY_OPERATOR && left->operator.type == typeop) {
        darray_append(left->operator.operands, right);
        t = left;
    } else if (right->category == TYPE_CATEGORY_OPERATOR && right->operator.type == typeop) {
        darray_prepend(right->operator.operands, left);
        t = right;
    } else if (!(t = module_types_set_has_str(m, type_op_create_str(left, right, typeop)))) {
        // else if the type [left OP right] is not already in the set create a new type
        t = type_alloc(m);
        if (!t) {
            RF_ERROR("Type allocation failed");
            return NULL;
        }
        t->category = TYPE_CATEGORY_OPERATOR;
        t->operator.type = typeop;
        darray_init(t->operator.operands);
        darray_append(t->operator.operands, left);
        darray_append(t->operator.operands, right);
        // since now we create a totally new type we should add it to the set
        if (!module_types_set_add(m, t, n)) {
            RF_ERROR("Failed to add a newly created type to the module's set of types");
            return NULL;
        }
    }
    return t;
}

/* -- various type creation and initialization functions -- */

struct type *type_create_from_node(const struct ast_node *node)
{
    switch (node->type) {
    case AST_XIDENTIFIER:
        return type_lookup_xidentifier(node);
    case AST_TYPE_DECLARATION:
        return type_create_from_typedecl(node);
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_OPERATOR:
    case AST_TYPE_LEAF:
        return type_create_from_typeelem(node);
    case AST_FUNCTION_DECLARATION:
        return type_create_from_fndecl(node);
    default:
        RF_ASSERT_OR_CRITICAL(
            false, return false,
            "Attempted to create a type for illegal ast node type \""RFS_PF"\"",
            RFS_PA(ast_node_str(node))
        );
    }

    return NULL;
}

struct type *type_create_from_typedecl(const struct ast_node *n)
{
    struct type *t;
    struct module *mod = type_creation_ctx_mod();
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);
    t = type_alloc(mod);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = TYPE_CATEGORY_DEFINED;
    t->defined.name = ast_typedecl_name_str(n);
    type_creation_ctx_set_genrdecl(ast_typedecl_genrdecl_get(n));
    t->defined.type = type_lookup_or_create(ast_typedecl_typedesc_get(n));
    if (!t->defined.type) {
        type_free(t, mod->types_pool);
        return NULL;
    }

    module_types_set_add(mod, t, n);
    return t;
}

static bool type_init_from_fndecl(struct type *t, const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    struct ast_node *args = ast_fndecl_args_get(n);
    struct ast_node *ret = ast_fndecl_return_get(n);
    struct type *arg_type = NULL;
    struct type *ret_type = NULL;

    // set argument type (left part of the operand)
    if (args) {
        type_creation_ctx_set_genrdecl(ast_fndecl_genrdecl_get(n));
        arg_type = type_lookup_or_create(args);
        if (!arg_type) {
            return false;
        }
        // also add the function's arguments to its symbol table if needed
        if (ast_fndecl_position_get(n) != FNDECL_PARTOF_FOREIGN_IMPORT) {
            struct function_args_ctx ctx;
            function_args_ctx_init(
                &ctx,
                ast_fndecl_symbol_table_get((struct ast_node*)n),
                type_creation_ctx_st(),
                type_creation_ctx_mod()
            );
            ast_type_foreach_leaf_arg(args, arg_type, (ast_type_cb)type_function_add_args_to_st, &ctx);
            if (!ctx.success) {
                RF_ERROR("Failed to add a function's arguments to its symbol table");
                return false;
            }
        }
    } else {
        arg_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
    }

    if (ret) {
        type_creation_ctx_set_genrdecl(ast_fndecl_genrdecl_get(n));
        ret_type = type_lookup_or_create(ret);
        if (!ret_type) {
            return false;
        }
    } else {
        ret_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
    }
    type_function_init(t, arg_type, ret_type);

    return true;
}

struct type *type_create_from_fndecl(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    struct type *t;
    struct module *m = type_creation_ctx_mod();

    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    if (!type_init_from_fndecl(t, n)) {
        RF_ERROR("Function type initialization failure");
        type_free(t, m->types_pool);
        return NULL;
    }

    module_types_set_add(m, t, n);
    return t;
}

struct type *type_simple_create(
    enum type_category category,
    const struct RFstring *name)
{
    struct module *m = type_creation_ctx_mod();
    struct type *t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }
    t->category = category;
    t->simple.name = name;
    return t;
}

static bool type_operator_init_from_node(struct type *t, const struct ast_node *n)
{
    bool ret = false;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);

    t->category = TYPE_CATEGORY_OPERATOR;
    darray_init(t->operator.operands);
    t->operator.type = ast_typeop_op(n);

    type_creation_ctx_push_op(&t->operator);
    struct type *left = type_lookup_or_create(ast_typeop_left(n));
    if (!left) {
        goto end;
    }
    struct type *right = type_lookup_or_create(ast_typeop_right(n));
    if (!right) {
        goto end;
    }

    // success
    ret = true;
end:
    type_creation_ctx_pop_op();
    return ret;
}

struct type *type_operator_create_from_node(struct ast_node *n)
{
    struct type *t;
    struct module *m = type_creation_ctx_mod();
    t = type_alloc(m);
    if (!t) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    if (!type_operator_init_from_node(t, n)) {
        type_free(t, m->types_pool);
        return NULL;
    }

    module_types_set_add(m, t, n);
    return t;
}

/* -- type getters -- */
struct type *type_lookup_xidentifier(const struct ast_node *n)
{
    const struct RFstring *id;
    struct type* ret;
    struct module *mod = type_creation_ctx_mod();

    AST_NODE_ASSERT_TYPE(n, AST_XIDENTIFIER);
    id = ast_xidentifier_str(n);

    if (!(ret = type_lookup_identifier_string(id, type_creation_ctx_st()))) {
        analyzer_err(
            type_creation_ctx_mod(), ast_node_startmark(n),
            ast_node_endmark(n),
            "Type \""RFS_PF"\" is not defined",
            RFS_PA(id)
        );
        return NULL;
    }

    // if not check if we have generic and if it is one of them
    struct ast_node *genrdecl = n->xidentifier.genr;
    if (genrdecl) {
        // TODO: read the generic type ast_node and create a generic type
        RF_ASSERT(false, "TODO: Not yet implemented");
        return NULL;
    }

    if (n->xidentifier.arrspec) {
        ret = type_array_get_or_create_from_ast(mod, n->xidentifier.arrspec, ret);
    }

    return ret;
}

struct type *type_lookup_identifier_string(
    const struct RFstring *str,
    const struct symbol_table *st
)
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
    if ((rec = symbol_table_lookup_record(st, str, NULL))) {
        return symbol_table_record_type(rec);
    }

    return NULL;
}

struct type *module_get_or_create_type(const struct ast_node *desc)
{
    struct type *t;
    if (desc->type == AST_TYPE_LEAF) {
        desc = ast_typeleaf_right(desc);
    }
    if (desc->type == AST_XIDENTIFIER) {
        return type_lookup_xidentifier(desc);
    }
    struct rf_objset_iter it;
    struct module *mod = type_creation_ctx_mod();
    rf_objset_foreach(mod->types_set, &it, t) {
        if (type_equals_ast_node(
                t,
                desc,
                mod,
                type_creation_ctx_st(),
                type_creation_ctx_genrdecl(),
                TYPECMP_IDENTICAL)) {
            return t;
        }
    }

    // else we have to create a new type
    if (!(t = type_create_from_node(desc))) {
        return NULL;
    }

    // TODO: Should it not have been added already by the proper creation function?
    // add it to the list
    module_types_set_add(mod, t, desc);
    return t;
}
