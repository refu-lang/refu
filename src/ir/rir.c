#include <ir/rir.h>

#include <Utils/memory.h>
#include <Utils/fixed_memory_pool.h>

#include <ast/ast.h>
#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>
#include <ir/elements.h>
#include <ir/rir_type.h>



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
    // transfer ownership of AST
    r->root = a->root;
    a->root = NULL;

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
    struct rir_type *t;
    ast_node_destroy(r->root);
    rf_fixed_memorypool_destroy(r->symbol_table_records_pool);
    rf_fixed_memorypool_destroy(r->types_pool);
    string_table_destroy(r->identifiers_table);
    string_table_destroy(r->string_literals_table);

    rf_ilist_for_each(&r->rir_types, t, ln) {
        rir_type_destroy(t);
    }
}

void rir_destroy(struct rir *r)
{
    rir_deinit(r);
    free(r);
}

struct rir_module *rir_process(struct rir *r)
{
    rir_create_types(&r->rir_types, &r->composite_types);
    // TODO: When modules are actually introduced change temporary module name
    //       to the name of the actual module being processed
    const struct RFstring mod_name = RF_STRING_STATIC_INIT("i_am_a_module");
    return rir_module_create(r->root, &mod_name);
}
