#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>
#include <Utils/hash.h>

#include <ast/ast.h>
#include <ast/identifier.h>


static size_t rehash_fn(const void *e, void *user_arg)
{
    // for now assume that all nodes are identifiers ... this
    // will probably not be the case
    return rf_hash_str_stable(ast_identifier_str(e), 0);
}

static bool cmp_fn(const void *e, void *str)
{
    // for now assume that all nodes are identifiers ... this
    // will probably not be the case
    return rf_string_equal(ast_identifier_str(e), str);
}

bool symbol_table_init(struct symbol_table *t)
{
    htable_init(&t->table, rehash_fn, NULL);
    return true;
}

void symbol_table_deinit(struct symbol_table *t)
{
    htable_clear(&t->table);
}

bool symbol_table_add(struct symbol_table *t,
                      const struct RFstring *id,
                      struct ast_node *n)
{
    // If it already exists in the symbol table return false
    if (NULL != symbol_table_lookup(t, id)) {
        return false;
    }

    if (!htable_add(&t->table, rf_hash_str_stable(id, 0), n)) {
        return false;
    }

    return true;
}

struct ast_node *symbol_table_lookup(struct symbol_table *t,
                                     const struct RFstring *id)
{
    return htable_get(&t->table, rf_hash_str_stable(id, 0), cmp_fn, id);
}
