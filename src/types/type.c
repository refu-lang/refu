#include <types/type.h>

#include <Persistent/buffers.h>
#include <Utils/fixed_memory_pool.h>
#include <Utils/bits.h>
#include <Definitions/threadspecific.h>

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
#include <types/type_comparisons.h>


const struct RFstring g_wildcard_s = RF_STRING_STATIC_INIT("_");
struct type_creation_ctx {
    //! A queue of type operators during creation
    struct {darray(struct type_operator*);} operators;
};
i_THREAD__ struct type_creation_ctx g_type_creation_ctx;
void type_creation_ctx_init()
{
    darray_init(g_type_creation_ctx.operators);
}

void type_creation_ctx_deinit()
{
    darray_clear(g_type_creation_ctx.operators);
    darray_free(g_type_creation_ctx.operators);
}

static inline void type_creation_ctx_push_op(struct type_operator *op)
{
    darray_append(g_type_creation_ctx.operators, op);
}

static inline void type_creation_ctx_pop_op()
{
    RF_ASSERT(darray_size(g_type_creation_ctx.operators) != 0, "Tried to pop non-existing typeop");
    (void)darray_pop(g_type_creation_ctx.operators);
}

static struct type_operator *type_creation_ctx_top_op()
{
    return darray_size(g_type_creation_ctx.operators) == 0
        ? NULL
        : darray_top(g_type_creation_ctx.operators);
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
static bool type_operator_init_from_node(struct type *t,
                                         const struct ast_node *n,
                                         struct module *m,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl);

/* -- miscellaneous type functions used internally (static) -- */
struct function_args_ctx {
    struct symbol_table *function_st;
    struct symbol_table *parent_st;
    struct module *module;
    bool success;
};

static inline void function_args_ctx_init(struct function_args_ctx *ctx,
                                          struct symbol_table *function_st,
                                          struct symbol_table *parent_st,
                                          struct module *m)
{
    ctx->function_st = function_st;
    ctx->parent_st = parent_st;
    ctx->module = m;
    ctx->success = true;
}

// Add all arguments to the symbol table. Leafs are added with key as their
// ID and anonymous types are added with a unique value as their key
static bool type_function_add_args_to_st(const struct RFstring *name,
                                         const struct ast_node *ast_desc,
                                         struct type *t,
                                         struct function_args_ctx *ctx)
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

void type_free(struct type *t, struct module *m)
{
    rf_fixed_memorypool_free_element(m->types_pool, t);
}

/* -- type creation and initialization functions used internally -- */
static bool type_init_from_typeelem(struct type *t,
                                    const struct ast_node *typeelem,
                                    struct module *m,
                                    struct symbol_table *st,
                                    struct ast_node *genrdecl)
{
    switch(typeelem->type) {
    case AST_TYPE_OPERATOR:
        return type_operator_init_from_node(t,
                                            typeelem,
                                            m,
                                            st,
                                            genrdecl);
    case AST_TYPE_DESCRIPTION:
        // case of anonymous type on the right of a typeleaf
        // e.g.:  foo:(i32 | string)
        return type_init_from_typeelem(t, ast_typedesc_desc_get(typeelem), m, st, genrdecl);
    default:
        RF_CRITICAL_FAIL("Case should never happen");
        break;
    }
    return false;
}

struct type *type_operator_lookup_or_create(const struct ast_node *opdesc,
                                            struct module *m,
                                            struct symbol_table *st,
                                            struct ast_node *genrdecl)
{
    struct type_operator *currop = type_creation_ctx_top_op();
    if (currop && currop->type == ast_typeop_op(opdesc)) {
        // if we are already inside a type operator of the same type
        struct type *left = type_lookup_or_create(ast_typeop_left(opdesc), m, st, genrdecl);
        if (!left) {
            RF_ERROR("Failed to find the left type of a typeop");
            return NULL;
        }
        struct type *right = type_lookup_or_create(ast_typeop_right(opdesc), m, st, genrdecl);
        if (!right) {
            RF_ERROR("Failed to find the right type of a typeop");
            return NULL;
        }
        return typeop_to_type(currop);
    } else {
        return module_get_or_create_type(m, opdesc, st, genrdecl);
    }
}

struct type *type_create_from_typeelem(const struct ast_node *typedesc,
                                       struct module *m,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl)
{
    struct type *ret;
    RF_ASSERT(typedesc->type != AST_TYPE_LEAF && typedesc->type != AST_XIDENTIFIER,
              "Typeleaf or identifier should never get here");

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

struct type *type_lookup_or_create(const struct ast_node *n,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl)
{
    struct type *ret = NULL;
    switch (n->type) {
    case AST_XIDENTIFIER:
        ret = type_lookup_xidentifier(n, m, st, genrdecl);
        break;
    case AST_TYPE_LEAF:
        ret = module_get_or_create_type(m, ast_typeleaf_right(n), st, genrdecl);
        break;
    case AST_TYPE_DESCRIPTION:
        return type_lookup_or_create(ast_typedesc_desc_get(n), m, st, genrdecl);
    case AST_TYPE_OPERATOR:
        ret = type_operator_lookup_or_create(n, m, st, genrdecl);
        break;
    case AST_VARIABLE_DECLARATION:
        return type_lookup_or_create(ast_vardecl_desc_get(n), m, st, genrdecl);
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Unexpected ast node type "
                              "\""RF_STR_PF_FMT"\" detected",
                              RF_STR_PF_ARG(ast_node_str(n)));
        break;
    }

    // we should get here only in the case of end type so add to current operator if existing
    if (ret) {
        type_add_to_currop(ret);
    }
    return ret;
}

struct type *type_create_from_operation(enum typeop_type typeop,
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
    } else { // create a new type

        // TODO: Somehow also check if the type is already existing in the analyzer
        //       and if it is return it instead of creating it
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
    }
    return t;
}

/* -- various type creation and initialization functions -- */

struct type *type_create_from_node(const struct ast_node *node,
                                   struct module *m,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl)
{
    switch (node->type) {
    case AST_XIDENTIFIER:
        return type_lookup_xidentifier(node, m, st, genrdecl);
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
                                            ast_typedecl_genrdecl_get(n));
    if (!t->defined.type) {
        type_free(t, m);
        return NULL;
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
        arg_type = type_lookup_or_create(args, m, st, ast_fndecl_genrdecl_get(n));
        if (!arg_type) {
            return false;
        }
        // also add the function's arguments to its symbol table if needed
        if (ast_fndecl_position_get(n) != FNDECL_PARTOF_FOREIGN_IMPORT) {
            struct function_args_ctx ctx;
            function_args_ctx_init(&ctx, ast_fndecl_symbol_table_get((struct ast_node*)n), st, m);
            ast_type_foreach_arg(args, arg_type, (ast_type_cb)type_function_add_args_to_st, &ctx);
            if (!ctx.success) {
                RF_ERROR("Failed to add a function's arguments to its symbol table");
                return false;
            }
        }
    } else {
        arg_type = (struct type*)type_elementary_get_type(ELEMENTARY_TYPE_NIL);
    }

    if (ret) {
        ret_type = type_lookup_or_create(ret, m, st,
                                         ast_fndecl_genrdecl_get(n));
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
        RF_ERROR("Function type initialization failure");
        type_free(t, m);
        return NULL;
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

static bool type_operator_init_from_node(struct type *t,
                                         const struct ast_node *n,
                                         struct module *m,
                                         struct symbol_table *st,
                                         struct ast_node *genrdecl)
{
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_OPERATOR);

    t->category = TYPE_CATEGORY_OPERATOR;
    darray_init(t->operator.operands);
    t->operator.type = ast_typeop_op(n);

    type_creation_ctx_push_op(&t->operator);
    struct type *left = type_lookup_or_create(ast_typeop_left(n), m, st, genrdecl);
    if (!left) {
        return false;
    }
    struct type *right = type_lookup_or_create(ast_typeop_right(n), m, st, genrdecl);
    if (!right) {
        return false;
    }
    type_creation_ctx_pop_op();

    return true;
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

    if (!type_operator_init_from_node(t, n, m, st, genrdecl)) {
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
        struct RFstring *ret = RFS("");
        size_t sz = 0;
        struct type **subt;
        darray_foreach(subt, t->operator.operands) {
            ret = RFS(RF_STR_PF_FMT RF_STR_PF_FMT, RF_STR_PF_ARG(ret), RF_STR_PF_ARG(type_str_do(*subt, options)));
            if (darray_size(t->operator.operands) - 1 != sz) {
                ret = RFS(RF_STR_PF_FMT RF_STR_PF_FMT, RF_STR_PF_ARG(ret), RF_STR_PF_ARG(type_op_str(t->operator.type)));
            }
            ++sz;
        }
        return ret;
    }
    case TYPE_CATEGORY_DEFINED:
        return RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(t->defined.name));
    case TYPE_CATEGORY_WILDCARD:
        return RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(&g_wildcard_s));
    default:
        RF_CRITICAL_FAIL("TODO: Not yet implemented");
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

i_INLINE_INS struct RFstring *type_str_or_die(const struct type *t, int options);

size_t type_get_uid(const struct type *t, bool count_leaf_id)
{
    size_t ret;
    struct RFstring *str;
    RFS_PUSH();
    str = count_leaf_id ? type_str_or_die(t, TSTR_DEFINED_CONTENTS)
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
    } else if (t->category == TYPE_CATEGORY_ELEMENTARY) {
        return type_elementary_get_str(t->elementary.etype);
    }
    return RFS_OR_DIE("internal_struct_%u", type_get_uid(t, count_leaf_id));
}

static struct type g_wildcard_type = {.category = TYPE_CATEGORY_WILDCARD};
const struct type *type_get_wildcard()
{
    return &g_wildcard_type;
}

i_INLINE_INS bool type_is_defined(const struct type *t);
i_INLINE_INS const struct RFstring *type_defined_get_name(const struct type *t);
i_INLINE_INS struct type *type_defined_get_type(const struct type *t);

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

void type_operator_add_operand(struct type_operator *p, struct type *c)
{
    if (p != &c->operator) {
        darray_append(p->operands, c);
    }
}
i_INLINE_INS void type_add_operand(struct type *p, struct type *c);

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

i_INLINE_INS bool type_is_operator(const struct type *t);
i_INLINE_INS struct type *typeop_to_type(struct type_operator *op);
i_INLINE_INS enum typeop_type type_typeop_get(const struct type *t);
i_INLINE_INS bool type_is_sumop(const struct type *t);
i_INLINE_INS bool type_is_prodop(const struct type *t);
i_INLINE_INS bool type_is_implop(const struct type *t);
i_INLINE_INS bool type_is_sumtype(const struct type *t);
