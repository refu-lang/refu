#include <ir/rir.h>

#include <Utils/memory.h>

#include <analyzer/analyzer.h>

bool rir_init(struct rir *r, struct analyzer *a)
{
    // transfer ownership of memory pools
    r->symbol_table_records_pool = a->symbol_table_records_pool;
    a->symbol_table_records_pool = NULL;
    r->types_pool = a->types_pool;
    a->types_pool = NULL;

    // transfer ownership of string tables
    r->identifiers_table = a->identifiers_table;
    a->identifiers_table = NULL;
    r->string_literals_table = a->string_literals_table;
    a->string_literals_table = NULL;

    // copy composite types list
    rf_ilist_copy(&a->composite_types, &r->composite_types);

    return true;
}

struct rir *rir_create(struct analyzer *a)
{
    struct rir *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_init(ret, a)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void rir_deinit(struct rir *r)
{
    (void)r;
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}
