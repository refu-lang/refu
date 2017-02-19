#include <analyzer/symbol_table.h>

#include <rfbase/string/core.h>
#include <rfbase/utils/hash.h>
#include <rfbase/utils/memory.h>
#include <rfbase/utils/fixed_memory_pool.h>

#include <module.h>
#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/type.h>
#include <ast/module.h>
#include <ast/function.h>
#include <ast/typeclass.h>
#include <types/type.h>
#include <types/type_function.h>
#include <analyzer/analyzer.h>
#include <ir/rir_object.h>

/* -- symbol table record related functions -- */

static struct symbol_table_record *symbol_table_record_alloc(
    struct symbol_table *st)
{
    return rf_fixed_memorypool_alloc_element(st->pool);
}

static void symbol_table_record_free(struct symbol_table_record *rec,
                                     struct symbol_table *st)
{
    rf_fixed_memorypool_free_element(st->pool, rec);
}

bool symbol_table_record_init(struct symbol_table_record *rec,
                              struct module *mod,
                              struct symbol_table *st,
                              const struct ast_node *node,
                              const struct RFstring *id)
{
    const struct RFstring *name = NULL;
    enum type_category category;
    switch (node->type) {
    case AST_MODULE:
        name = ast_module_name(node);
        category = TYPE_CATEGORY_MODULE;
        break;
    case AST_TYPECLASS_DECLARATION:
        name = ast_typeclass_name_str(node);
        category = TYPE_CATEGORY_TYPECLASS;
        break;
    case AST_TYPECLASS_INSTANCE:
        name = ast_typeclass_name_str(node);
        category = TYPE_CATEGORY_TYPEINSTANCE;
        break;
    default:
        break;
    }

    RF_STRUCT_ZERO(rec);
    rec->node = node;
    rec->id = id;
    switch (node->type) {
    case AST_MODULE:
    case AST_TYPECLASS_DECLARATION:
    case AST_TYPECLASS_INSTANCE:
        type_creation_ctx_set_args(mod, st, NULL);
        rec->data = type_simple_create(category, id);
        break;
    case AST_FUNCTION_DECLARATION:
        type_creation_ctx_set_args(mod, st, NULL);
        rec->data = type_create_from_fndecl(node);
        break;
    case AST_TYPE_DECLARATION:
        type_creation_ctx_set_args(mod, st, NULL);
        rec->data = type_create_from_typedecl(node);
        break;
    case AST_VARIABLE_DECLARATION:
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_LEAF:
        type_creation_ctx_set_args(mod, st, NULL);
        rec->data = type_lookup_or_create(node);
        break;
    case AST_IDENTIFIER:
        // we can only come here if the identifier's type is already determined
        // e.g. by type inference, say on the loop variable of a for expression
        RF_ASSERT(
            ast_node_get_type(node),
            "Attempting to create a symbol table record with an identifier "
            "whose type has not yet been determined."
        );
        rec->data = (struct type*) ast_node_get_type(node);
        break;
    default:
        RF_ASSERT_OR_CRITICAL(
            false, return false,
            "Attempted to create symbol table record "
            "for illegal ast node type \""RFS_PF"\"",
            RFS_PA(ast_node_str(node))
        );
        break;
    }

    if (!rec->data) {
        return false;
    }

    if (node->type != AST_TYPE_DECLARATION && type_is_defined(rec->data)) {
        // make sure that all records whose type is a type declaration point to
        // the correct ast type declaration node
        struct symbol_table_record *r = symbol_table_lookup_record(
            st, type_defined_get_name(rec->data), NULL
        );
        RF_ASSERT(r, "A record for the typedef should already exist by this point");
        rec->node = r->node;
    }

    return true;
}

static bool symbol_table_record_init_from_type(struct symbol_table_record *rec,
                                               const struct RFstring *id,
                                               struct type *t)
{
    RF_STRUCT_ZERO(rec);
    rec->id = id;
    rec->data = t;
    return true;
}

struct symbol_table_record *symbol_table_record_create(
    struct symbol_table *st,
    struct module *mod,
    const struct ast_node *node,
    const struct RFstring *id)
{
    struct symbol_table_record *ret = symbol_table_record_alloc(st);
    if (!ret) {
        RF_ERROR("Failed to allocate a symbol table record");
        return NULL;
    }

    if (!symbol_table_record_init(ret, mod, st, node, id)) {
        symbol_table_record_destroy(ret, st);
        return NULL;
    }

    return ret;
}

struct symbol_table_record *symbol_table_record_create_from_type(
    struct symbol_table *st,
    const struct RFstring *id,
    struct type *t)
{
    struct symbol_table_record *ret;
    ret = symbol_table_record_alloc(st);
    if (!ret) {
        RF_ERROR("Failed to allocate a symbol table record");
        return NULL;
    }

    if (!symbol_table_record_init_from_type(ret, id, t)) {
        symbol_table_record_destroy(ret, st);
        return NULL;
    }

    return ret;
}

void symbol_table_record_destroy(struct symbol_table_record *rec,
                                 struct symbol_table *st)
{
    symbol_table_record_free(rec, st);
}

i_INLINE_INS const struct RFstring *
symbol_table_record_id(struct symbol_table_record *rec);
i_INLINE_INS const struct ast_node *
symbol_table_record_node(struct symbol_table_record *rec);
i_INLINE_INS struct type *
symbol_table_record_type(struct symbol_table_record *rec);

/* -- symbol table related functions -- */


static size_t rehash_fn(const void *e, void *user_arg)
{
    struct symbol_table_record *rec = (struct symbol_table_record*)e;
    return rf_hash_str_stable(rec->id, 0);
}

static bool cmp_fn(const void *e, void *str)
{
    struct symbol_table_record *rec = (struct symbol_table_record*)e;
    return rf_string_equal(rec->id, str);
}

bool symbol_table_init(struct symbol_table *t, struct module *m)
{
    RF_STRUCT_ZERO(t);
    htable_init(&t->table, rehash_fn, NULL);
    t->pool = m->symbol_table_records_pool;
    t->mod = m;
    return true;
}

bool root_symbol_table_init(struct symbol_table *t)
{
    RF_STRUCT_ZERO(t);
    htable_init(&t->table, rehash_fn, NULL);
    t->mod = NULL;
    t->pool = rf_fixed_memorypool_create(
        sizeof(struct symbol_table_record),
        RECORDS_TABLE_POOL_CHUNK_SIZE
    );
    return t->pool;
}

void symbol_table_deinit(struct symbol_table *t)
{
    // free memory of all symbol table records
    symbol_table_iterate(t, (htable_iter_cb)symbol_table_record_destroy, t);
    // clear the table
    htable_clear(&t->table);
    if (symbol_table_is_root(t)) {
        rf_fixed_memorypool_destroy(t->pool);
    }
}

bool symbol_table_add_node(
    struct symbol_table *t,
    struct module *mod,
    const struct RFstring *id,
    struct ast_node *n)
{
    struct symbol_table_record *rec;
    bool at_first;

    // this check may be redundant due to the way symbol tables are
    // created in symbol_table_creation. BUT better safe than sorry
    rec = symbol_table_lookup_record(t, id, &at_first);
    RF_ASSERT_OR_CRITICAL(!(rec && at_first), return false,
                          "Attempted to add an already existing node"
                          " to a symol table");


    rec = symbol_table_record_create(t, mod, n, id);
    if (!rec) {
        return false;
    }


    if (!symbol_table_add_record(t, rec)) {
        symbol_table_record_destroy(rec, t);
        return false;
    }
    // set the type in the node
    n->expression_type = rec->data;
    
    return true;
}

bool symbol_table_add_record(struct symbol_table *t,
                             struct symbol_table_record *rec)
{
    if (!htable_add(&t->table, rf_hash_str_stable(rec->id, 0), rec)) {
        return false;
    }

    return true;
}

bool symbol_table_add_type(struct symbol_table *st,
                           struct module *mod,
                           const struct RFstring *id,
                           struct type *t,
                           const struct ast_node *node_desc)
{
    struct symbol_table_record *rec;
    bool symbol_found_at_first_st;

    rec = symbol_table_lookup_record(st, id, &symbol_found_at_first_st);

    if (rec && symbol_found_at_first_st) {
        analyzer_err(mod, ast_node_startmark(rec->node),
                     ast_node_endmark(rec->node),
                     "Identifier \""RFS_PF"\" was already declared in scope "
                     "at "INPLOCATION_FMT,
                     RFS_PA(id),
                     INPLOCATION_ARG(module_get_file(mod),
                                     ast_node_location(rec->node)));
        return false;
    }

    //we can create the type now
    rec = symbol_table_record_create_from_type(st, id, t);
    if (!rec) {
        return false;
    }
    rec->node = node_desc;

    if (!symbol_table_add_record(st, rec)) {
        symbol_table_record_destroy(rec, st);
        return false;
    }

    return true;
}

void symbol_table_record_print(const struct symbol_table_record *rec)
{
    printf("Symbol table record\n");
    if (rec->id) {
        printf("id: " RFS_PF"\n", RFS_PA(rec->id));
    }
    if (rec->node) {
        printf(
            "node: %p \""RFS_PF"\"\n",
            rec->node,
            RFS_PA(ast_node_str(rec->node))
        );
    }
    RFS_PUSH();
    if (rec->data) {
        printf("type: "RFS_PF"\n", RFS_PA(type_str(rec->data, TSTR_DEFAULT)));
    }
    if (rec->rirobj) {
        printf(
            "rir_object: %p \""RFS_PF"\"\n",
            rec->rirobj,
            RFS_PA(rir_object_string(rec->rirobj))
        );
    }
    RFS_POP();
}

struct symbol_table_record *symbol_table_lookup_record(const struct symbol_table *t,
                                                       const struct RFstring *id,
                                                       bool *at_first_symbol_table)
{
    struct symbol_table_record *rec;
    const struct symbol_table *lp_table = t;

    if (at_first_symbol_table) {
        *at_first_symbol_table = false;
    }

    // search this symbol table
    rec = htable_get(&t->table, rf_hash_str_stable(id, 0), cmp_fn, id);
    if (rec) {
        if (at_first_symbol_table) {
            *at_first_symbol_table = true;
        }
        return rec;
    }
    // search all parents until we get to root
    while (!rec && lp_table->parent) {
        lp_table = lp_table->parent;
        rec = htable_get(&lp_table->table, rf_hash_str_stable(id, 0), cmp_fn, id);
    }

    // if we reach the root and we got nothing then check modules we depend on
    // TODO: this is not very well written. Don't like how I have an exception for
    //       module names here. Also need to think how to handle specific inclusions
    //       from modules.
    if (!rec && t->mod) {
        struct module **mod;
        darray_foreach(mod, t->mod->dependencies) {
            if ((rec = symbol_table_lookup_record(module_symbol_table(*mod), id, NULL)) &&
                !rf_string_equal(id, module_name(*mod))) {
                return rec;
            }
            rec = NULL;
        }
    }
    
    return rec;
}


struct symbol_table_record *symbol_table_lookup_rirobj(const struct symbol_table *t,
                                                       struct rir_object *obj)
{
    struct symbol_table_record *rec;
    const struct symbol_table *lp_table = t;

    // if we have an ID it can be a normal lookup
    if (obj->category == RIR_OBJ_EXPRESSION &&
        obj->expr.type == RIR_EXPRESSION_ALLOCA &&
        obj->expr.alloca.ast_id != NULL) {

        return symbol_table_lookup_record(t, obj->expr.alloca.ast_id, NULL);
    }

    // without a string ID we are forced to iterate :(
    do {
        struct htable_iter it;
        htable_foreach(&lp_table->table, &it, rec) {
            if (rec->rirobj == obj) {
                return rec;
            }
        }
    } while((lp_table = lp_table->parent) != NULL);

    // if we reach the root and we got nothing then check modules we depend on
    // TODO: this is not very well written. Don't like how I have an exception for
    //       module names here. Also need to think how to handle specific inclusions
    //       from modules.
    if (t->mod) {
        struct module **mod;
        darray_foreach(mod, t->mod->dependencies) {
            if ((rec = symbol_table_lookup_rirobj(module_symbol_table(*mod), obj))) {
                return rec;
            }
            rec = NULL;
        }
    }

    return rec;
}

// This function should be enclosed in RFS_PUSH() / RFS_POP()
static const struct RFstring *symbol_table_extract_string_from_typedesc(const struct ast_node *typedesc)
{
    if (typedesc->type == AST_TYPE_DESCRIPTION) {
        return symbol_table_extract_string_from_typedesc(typedesc->typedesc.desc);
    } else if (typedesc->type == AST_TYPE_LEAF) {
        return ast_typeleaf_str(typedesc);
    } else if (typedesc->type == AST_TYPE_OPERATOR) {
        return type_get_unique_type_str(ast_node_get_type(typedesc));
    }
    return NULL;
}

struct symbol_table_record *symbol_table_lookup_typedesc(const struct symbol_table *t,
                                                         const struct ast_node *typedesc,
                                                         bool *at_first_symbol_table)
{
    RFS_PUSH();
    const struct RFstring *id = symbol_table_extract_string_from_typedesc(typedesc);
    struct symbol_table_record *rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    RFS_POP();
    return rec;
}

struct type *symbol_table_lookup_defined_type(const struct symbol_table *t,
                                              const struct RFstring *id,
                                              bool *at_first_symbol_table)
{
    struct symbol_table_record *rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    return (rec && rec->data && rec->data->category == TYPE_CATEGORY_DEFINED)
        ? rec->data : NULL;
}

const struct ast_node *symbol_table_lookup_node(struct symbol_table *t,
                                                const struct RFstring *id,
                                                bool *at_first_symbol_table)
{
    struct symbol_table_record *rec;
    rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    return rec ? rec->node : NULL;
}

void symbol_table_iterate(struct symbol_table *t, htable_iter_cb cb, void *user)
{
    htable_iterate_records(&t->table, cb, user);
}

#ifdef RF_OPTION_DEBUG
void symbol_table_print(struct symbol_table *t)
{
    if (symbol_table_is_empty(t)) {
        return;
    }
    printf("\nPrinting records of a symbol table\n\n");
    htable_iterate_records(&t->table, (htable_iter_cb)symbol_table_record_print, NULL);
    printf("\n");
    fflush(stdout);
}
#endif

i_INLINE_INS void symbol_table_set_parent(struct symbol_table *t,
                                          struct symbol_table *parent);
i_INLINE_INS void symbol_table_set_fndecl(struct symbol_table *t,
                                           struct ast_node *decl);
i_INLINE_INS struct ast_node *symbol_table_get_fndecl(struct symbol_table *t);
i_INLINE_INS bool symbol_table_is_root(const struct symbol_table *t);
i_INLINE_INS bool symbol_table_is_empty(const struct symbol_table *t);
i_INLINE_INS void symbol_table_swap_current(struct symbol_table **current_st_ptr,
                                            struct symbol_table *new_st);
i_INLINE_INS struct type *symbol_table_lookup_type(struct symbol_table *t,
                                                   const struct RFstring *id,
                                                   bool *at_first_symbol_table);
