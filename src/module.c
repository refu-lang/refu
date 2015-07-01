#include <module.h>

#include <Utils/fixed_memory_pool.h>

#include <compiler.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/module.h>
#include <ast/type.h>
#include <ast/ast_utils.h>
#include <types/type.h>
#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>
#include <analyzer/analyzer_pass1.h>
#include <analyzer/typecheck.h>
#include <types/type_comparisons.h>

static const struct RFstring g_main_module_str = RF_STRING_STATIC_INIT("main");

static bool module_init(struct module *m, struct ast_node *n, struct front_ctx *front)
{
    // initialize
    RF_STRUCT_ZERO(m);
    m->node = n;
    m->front = front;
    darray_init(m->dependencies);
    // add to the compiler's modules
    darray_append(compiler_instance_get()->modules, m);

    // initialize analysis related members
    m->symbol_table_records_pool = rf_fixed_memorypool_create(sizeof(struct symbol_table_record),
                                                              RECORDS_TABLE_POOL_CHUNK_SIZE);
    if (!m->symbol_table_records_pool) {
        RF_ERROR("Failed to initialize a fixed memory pool for symbol records");
        return false;
    }
    m->types_pool = rf_fixed_memorypool_create(sizeof(struct type),
                                               TYPES_POOL_CHUNK_SIZE);
    if (!m->types_pool) {
        RF_ERROR("Failed to initialize a fixed memory pool for types");
        return false;
    }

    RF_MALLOC(m->types_set, sizeof(*m->types_set), return false);
    rf_objset_init(m->types_set, type);

    if (!(m->identifiers_table = string_table_create())) {
        RF_ERROR("Failed to allocate a string table for identifiers");
        return false;
    }
    if (!(m->string_literals_table = string_table_create())) {
        RF_ERROR("Failed to allocate a string table for string literals");
        return false;
    }
    
    return true;
}

struct module *module_create(struct ast_node *n, struct front_ctx *front)
{
    struct module *ret;
    RF_ASSERT(n->type == AST_MODULE || n->type == AST_ROOT,
              "Unexpected ast node type");
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!module_init(ret, n, front)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void module_deinit(struct module *m)
{
   if (m->symbol_table_records_pool) {
        rf_fixed_memorypool_destroy(m->symbol_table_records_pool);
    }
    if (m->types_pool) {
        rf_fixed_memorypool_destroy(m->types_pool);
    }
    if (m->identifiers_table) {
        string_table_destroy(m->identifiers_table);
    }
    if (m->string_literals_table) {
        string_table_destroy(m->string_literals_table);
    }

    if (m->types_set) {
        rf_objset_clear(m->types_set);
        free(m->types_set);
    }

    if (m->rir_types_list) {
        rir_types_list_destroy(m->rir_types_list);
    }
    darray_free(m->dependencies);
}

void module_destroy(struct module* m)
{
    module_deinit(m);
    free(m);
}

bool module_add_import(struct module *m, struct ast_node *import)
{
    AST_NODE_ASSERT_TYPE(import, AST_IMPORT);
    struct ast_node *c;
    struct module *other_mod;
    rf_ilist_for_each(&import->children, c, lh) {

        other_mod = compiler_module_get(ast_identifier_str(c));
        if (!other_mod) {
            // requested import module not found
            return false;
        }
        darray_append(m->dependencies, other_mod);
    }
    return true;
}

const struct RFstring *module_name(const struct module *m)
{
    return m->node->type == AST_ROOT ? &g_main_module_str : ast_module_name(m->node);
}

struct symbol_table *module_symbol_table(const struct module *m)
{
    return ast_node_symbol_table_get(m->node);
}

bool module_symbol_table_init(struct module *m)
{
    RF_ASSERT(m->node->type == AST_ROOT || m->node->type == AST_MODULE,
              "Illegal ast node detected");
        return m->node->type == AST_ROOT
            ? true  // the root symbol table should have already been initialized
            : symbol_table_init(&m->node->module.st, m);
}

struct inpfile *module_get_file(const struct module *m)
{
    return m->front->info->file;
}

bool module_is_main(const struct module *m)
{
    return rf_string_equal(module_name(m), &g_main_module_str);
}

bool module_add_stdlib(struct module *m)
{
    static const struct RFstring stdlib_s = RF_STRING_STATIC_INIT("stdlib");
    struct module *other_mod = compiler_module_get(&stdlib_s);
    if (!other_mod) {
        RF_ERROR("stdlib was requested but could not be found in the parsed compiler modules");
        return false;
    }
    darray_append(m->dependencies, other_mod);
    return true;
}


bool module_types_set_add(struct module *m, struct type *new_type)
{
    return rf_objset_add(m->types_set, type, new_type);
}

struct type *module_get_or_create_type(struct module *mod,
                                       const struct ast_node *desc,
                                       struct symbol_table *st,
                                       struct ast_node *genrdecl)
{
    struct type *t;
    struct rf_objset_iter it;
    RF_ASSERT(desc->type == AST_TYPE_DESCRIPTION ||
              desc->type == AST_TYPE_OPERATOR ||
              desc->type == AST_TYPE_LEAF,
              "Unexpected ast node type");
    rf_objset_foreach(mod->types_set, &it, t) {
        if (type_equals_ast_node(t, desc, mod, st, genrdecl, TYPECMP_GENERIC)) {
            return t;
        }
    }

    // else we have to create a new type
    t = type_create_from_node(desc, mod, st, genrdecl);
    if (!t) {
        RF_ERROR("Failure to create a composite type");
        return NULL;
    }

    // add it to the list
    module_types_set_add(mod, t);
    if (desc->type == AST_TYPE_OPERATOR && ast_typeop_op(desc) == TYPEOP_SUM) {
        // if it's a sum type also add the left and the right operand type
        module_types_set_add(mod, t->operator.left);
        module_types_set_add(mod, t->operator.right);
    }
    return t;
}


static bool module_determine_dependencies_do(struct ast_node *n, void *user_arg)
{
    struct module *mod = user_arg;
    switch (n->type) {
    case AST_IMPORT:
        if (!ast_import_is_foreign(n)) {
            return module_add_import(mod, n);
        }
    default:
        break;
    }
    return true;
}

bool module_determine_dependencies(struct module *m, bool use_stdlib)
{
    // initialize module symbol table here instead of analyzer_first_pass
    // since we need it beforehand to get symbols from import
    if (!module_symbol_table_init(m)) {
        RF_ERROR("Could not initialize symbol table for root node");
        return false;
    }

    // read the imports and add dependencies
    if (!ast_pre_traverse_tree(m->node, module_determine_dependencies_do, m)) {
        return false;
    }

    // TODO: This can't be the best way to achieve this. Rethink when possible
    // if this is the main module add the stdlib as dependency,
    // unless a program without the stdlib was requested
    if (use_stdlib && module_is_main(m)) {
        return module_add_stdlib(m);
    }
    return true;
}

bool module_analyze(struct module *m)
{
    // create symbol tables and change ast nodes ownership
    if (!analyzer_first_pass(m)) {
        RF_ERROR("Failure at module analysis first pass");
        return false;
    }

    if (!analyzer_typecheck(m, m->node)) {
        RF_ERROR("Failure at module's typechecking");
        return false;
    }

    if (!analyzer_finalize(m)) {
        RF_ERROR("Failure at module's finalization");
        return false;
    }
    return true;
}
