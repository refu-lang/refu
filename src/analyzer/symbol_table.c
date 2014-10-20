#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>
#include <Utils/memory.h>

#include <ast/ast.h>
#include <ast/identifier.h>
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

void symbol_table_record_init(struct symbol_table_record *rec,
                              struct ast_node *type,
                              const struct RFstring *id)
{
    rec->type = type;
    rec->id = id;
}

struct symbol_table_record *symbol_table_record_create(struct symbol_table *st,
                                                       struct ast_node *type,
                                                       const struct RFstring *id)
{
    struct symbol_table_record *ret;
    ret = symbol_table_record_alloc(st);
    if (!ret) {
        RF_ERROR("Failed to allocate a symbol table record");
        return NULL;
    }
    symbol_table_record_init(ret, type, id);
    return ret;
}

void symbol_table_record_destroy(struct symbol_table_record *rec,
                                 struct symbol_table *st)
{
    //TODO: if needing to deinit anything create a XXX_deinit() function
    symbol_table_record_free(rec, st);
}

i_INLINE_INS const struct RFstring *
symbol_table_record_id(struct symbol_table_record *rec);
i_INLINE_INS struct ast_node *
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

bool symbol_table_init(struct symbol_table *t, struct analyzer *a)
{
    htable_init(&t->table, rehash_fn, NULL);
    t->parent = NULL;
    t->pool = &a->symbol_table_records_pool;
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
                           const struct RFstring *id,
                           struct ast_node *n)
{
    struct symbol_table_record *rec;
    rec = symbol_table_record_create(t, n, id);
    if (!rec) {
        return false;
    }

    if (!symbol_table_add_record(t, rec)) {
        symbol_table_record_destroy(rec, t);
        return false;
    }

    return true;
}

bool symbol_table_add_record(struct symbol_table *t,
                             struct symbol_table_record *rec)
{
    if (NULL != symbol_table_lookup_record(t, rec->id)) {
        return false;
    }

    if (!htable_add(&t->table, rf_hash_str_stable(rec->id, 0), rec)) {
        return false;
    }

    return true;
}

struct symbol_table_record *symbol_table_lookup_record(struct symbol_table *t,
                                                       const struct RFstring *id)
{
    return htable_get(&t->table, rf_hash_str_stable(id, 0), cmp_fn, id);
}

struct ast_node *symbol_table_lookup_node(struct symbol_table *t,
                                          const struct RFstring *id)
{
    struct symbol_table_record *rec;
    rec = symbol_table_lookup_record(t, id);
    return rec->type;
}

void symbol_table_iterate(struct symbol_table *t, htable_iter_cb cb, void *user)
{
    htable_iterate_values(&t->table, cb, user);
}

i_INLINE_INS void symbol_table_set_parent(struct symbol_table *t,
                                          struct symbol_table *parent);
