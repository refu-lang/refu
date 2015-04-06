#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>
#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/type.h>
#include <types/type.h>
#include <analyzer/analyzer.h>

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
                              struct analyzer *analyzer,
                              struct symbol_table *st,
                              struct ast_node *node,
                              const struct RFstring *id)
{
    rec->node = node;
    rec->id = id;
    rec->rir_data = NULL;
    rec->backend_handle = NULL;
    switch (node->type) {
    case AST_FUNCTION_DECLARATION:
        rec->data = type_create_from_fndecl(node, analyzer, st);
        break;
    case AST_TYPE_DECLARATION:
        rec->data = type_create_from_typedecl(node, analyzer, st);
        break;
    case AST_VARIABLE_DECLARATION:
    case AST_TYPE_DESCRIPTION:
    case AST_TYPE_LEAF:
        rec->data = type_lookup_or_create(node, analyzer, st, NULL, false);
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, return false, "Attempted to create symbol table record "
                              "for illegal ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(node)));
    }

    if (!rec->data) {
        return false;
    }

    return true;
}

bool symbol_table_record_init_from_type(struct symbol_table_record *rec,
                                        const struct RFstring *id,
                                        struct type *t)
{
    rec->node = NULL;
    rec->id = id;
    rec->data = t;
    return true;
}

struct symbol_table_record *symbol_table_record_create(struct symbol_table *st,
                                                       struct analyzer *analyzer,
                                                       struct ast_node *node,
                                                       const struct RFstring *id)
{
    struct symbol_table_record *ret;
    ret = symbol_table_record_alloc(st);
    if (!ret) {
        RF_ERROR("Failed to allocate a symbol table record");
        return NULL;
    }

    if (!symbol_table_record_init(ret, analyzer, st, node, id)) {
        symbol_table_record_destroy(ret, st);
        return NULL;
    }

    return ret;
}

struct symbol_table_record *symbol_table_record_create_from_type(
    struct symbol_table *st,
    struct analyzer *analyzer,
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
i_INLINE_INS enum type_category
symbol_table_record_category(struct symbol_table_record *rec);
i_INLINE_INS struct ast_node *
symbol_table_record_node(struct symbol_table_record *rec);
i_INLINE_INS struct type *
symbol_table_record_type(struct symbol_table_record *rec);
i_INLINE_INS struct rir_type *
symbol_table_record_rir_type(struct symbol_table_record *rec);
i_INLINE_INS void *
symbol_table_record_get_backend_handle(struct symbol_table_record *rec);
i_INLINE_INS void
symbol_table_record_set_backend_handle(struct symbol_table_record *rec, void *handle);

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

bool symbol_table_init(struct symbol_table *t, struct analyzer *a)
{
    RF_STRUCT_ZERO(t);
    htable_init(&t->table, rehash_fn, NULL);
    t->pool = a->symbol_table_records_pool;
    return true;
}

void symbol_table_deinit(struct symbol_table *t)
{
    // free memory of all symbol table records
    symbol_table_iterate(t, (htable_iter_cb)symbol_table_record_destroy, t);
    // clear the table
    htable_clear(&t->table);
}

bool symbol_table_add_node(struct symbol_table *t,
                           struct analyzer *analyzer,
                           const struct RFstring *id,
                           struct ast_node *n)
{
    struct symbol_table_record *rec;
    bool at_first;

    // this check may be redundant due to the way symbol tables are
    // created in symbol_table_creation.c BUT better safe than sorry
    rec = symbol_table_lookup_record(t, id, &at_first);
    RF_ASSERT_OR_CRITICAL(!(rec && at_first), return false,
                          "Attempted to add an already existing node"
                          " to a symol table");


    rec = symbol_table_record_create(t, analyzer, n, id);
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

bool symbol_table_add_typedecl(struct symbol_table *st,
                               struct analyzer *analyzer,
                               struct ast_node *n)
{
    struct ast_node *search_node;
    struct symbol_table_record *rec;
    bool symbol_found_at_first_st;
    const struct RFstring *type_name;
    AST_NODE_ASSERT_TYPE(n, AST_TYPE_DECLARATION);

    type_name = ast_typedecl_name_str(n);
    search_node = symbol_table_lookup_node(st,
                                           type_name,
                                           &symbol_found_at_first_st);

    if (search_node && symbol_found_at_first_st) {
        analyzer_err(analyzer, ast_node_startmark(search_node),
                     ast_node_endmark(search_node),
                     "Type \""RF_STR_PF_FMT"\" was already declared in scope "
                     "at "INPLOCATION_FMT,
                     RF_STR_PF_ARG(type_name),
                     INPLOCATION_ARG(analyzer_get_file(analyzer),
                                     ast_node_location(search_node)));
        return false;
    }

    //we can create the type now
    rec = symbol_table_record_create(st, analyzer, n, type_name);
    if (!rec) {
        return false;
    }

    return true;
}

bool symbol_table_add_type(struct symbol_table *st,
                           struct analyzer *analyzer,
                           const struct RFstring *id,
                           struct type *t)
{
    struct symbol_table_record *rec;
    bool symbol_found_at_first_st;


    rec = symbol_table_lookup_record(st, id, &symbol_found_at_first_st);

    if (rec && symbol_found_at_first_st) {
        analyzer_err(analyzer, ast_node_startmark(rec->node),
                     ast_node_endmark(rec->node),
                     "Identifier \""RF_STR_PF_FMT"\" was already declared in scope "
                     "at "INPLOCATION_FMT,
                     RF_STR_PF_ARG(id),
                     INPLOCATION_ARG(analyzer_get_file(analyzer),
                                     ast_node_location(rec->node)));
        return false;
    }

    //we can create the type now
    rec = symbol_table_record_create_from_type(st, analyzer, id, t);
    if (!rec) {
        return false;
    }

    if (!symbol_table_add_record(st, rec)) {
        symbol_table_record_destroy(rec, st);
        return false;
    }

    return true;
}


struct symbol_table_record *symbol_table_lookup_record(struct symbol_table *t,
                                                       const struct RFstring *id,
                                                       bool *at_first_symbol_table)
{
    struct symbol_table_record *rec;
    struct symbol_table *lp_table = t;

    if (at_first_symbol_table) {
        *at_first_symbol_table = false;
    }

    rec = htable_get(&t->table, rf_hash_str_stable(id, 0), cmp_fn, id);
    if (rec) {
        if (at_first_symbol_table) {
            *at_first_symbol_table = true;
        }
        return rec;
    }

    while (!rec && lp_table->parent) {
        lp_table = lp_table->parent;
        rec = htable_get(&lp_table->table, rf_hash_str_stable(id, 0), cmp_fn, id);
    }
    return rec;
}

struct ast_node *symbol_table_lookup_node(struct symbol_table *t,
                                          const struct RFstring *id,
                                          bool *at_first_symbol_table)
{
    struct symbol_table_record *rec;
    rec = symbol_table_lookup_record(t, id, at_first_symbol_table);
    if (!rec) {
        return NULL;
    }
    return rec ? rec->node : NULL;
}

void symbol_table_iterate(struct symbol_table *t, htable_iter_cb cb, void *user)
{
    htable_iterate_records(&t->table, cb, user);
}

i_INLINE_INS void symbol_table_set_parent(struct symbol_table *t,
                                          struct symbol_table *parent);
i_INLINE_INS void symbol_table_set_fndecl(struct symbol_table *t,
                                           struct ast_node *decl);
i_INLINE_INS struct ast_node *symbol_table_get_fndecl(struct symbol_table *t);
i_INLINE_INS void symbol_table_swap_current(struct symbol_table **current_st_ptr,
                                            struct symbol_table *new_st);
i_INLINE_INS struct type *symbol_table_lookup_type(struct symbol_table *t,
                                                   const struct RFstring *id,
                                                   bool *at_first_symbol_table);
