#include <module.h>

#include <rfbase/utils/fixed_memory_pool.h>

#include <utils/common_strings.h>
#include <compiler.h>
#include <utils/common_strings.h>
#include <parser/parser_common.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/module.h>
#include <ast/type.h>
#include <ast/ast_utils.h>
#include <types/type.h>
#include <types/type_comparisons.h>
#include <analyzer/analyzer.h>
#include <analyzer/analyzer_pass1.h>
#include <analyzer/typecheck.h>
#include <ir/rir.h>

static bool module_init(
    struct module *m,
    struct ast_node *n,
    struct rir *rir,
    struct front_ctx *front
)
{
    RF_ASSERT(!n || (n->type == AST_MODULE || n->type == AST_ROOT),
              "Either no ast node, or module/ast_root expected");
    // initialize
    RF_STRUCT_ZERO(m);
    m->node = n;
    m->rir = rir;
    m->front = front;
    darray_init(m->dependencies);
    darray_init(m->foreignfn_arr);
    darray_init(m->instantiated_typeclasses);
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

    rf_objset_init(&m->identifiers_set, string);
    rf_objset_init(&m->string_literals_set, string);

    return true;
}

struct module *module_create(struct ast_node *n, struct rir *rir, struct front_ctx *front)
{
    struct module *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!module_init(ret, n, rir, front)) {
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
    rf_objset_clear(&m->identifiers_set);
    rf_objset_clear(&m->string_literals_set);

    if (m->types_set) {
        type_objset_destroy(m->types_set, m->types_pool);
    }
    if (m->types_pool) {
        rf_fixed_memorypool_destroy(m->types_pool);
    }

    if (m->rir) {
        rir_destroy(m->rir);
    }

    darray_free(m->foreignfn_arr);
    darray_free(m->instantiated_typeclasses);
    darray_free(m->dependencies);
}

void module_destroy(struct module* m)
{
    module_deinit(m);
    free(m);
}

void module_add_foreign_import(struct module *m, struct ast_node *import)
{
    RF_ASSERT(
        ast_node_is_foreign_import(import),
        "Expected a foreign import node"
    );
    struct ast_node **child;
    darray_foreach(child, import->children) {
        // for now foreign import should only import function decls
        AST_NODE_ASSERT_TYPE(*child, AST_FUNCTION_DECLARATION);
        darray_append(m->foreignfn_arr, *child);
    }
}

void module_add_type_instance(struct module *m, struct ast_node *typeinstance)
{
    AST_NODE_ASSERT_TYPE(typeinstance, AST_TYPECLASS_INSTANCE);
    darray_append(m->instantiated_typeclasses, typeinstance);
}

bool module_add_import(struct module *m, struct ast_node *import)
{
    AST_NODE_ASSERT_TYPE(import, AST_IMPORT);
    struct ast_node **c;
    struct module *other_mod;
    darray_foreach(c, import->children) {

        other_mod = compiler_module_get(ast_identifier_str(*c));
        if (!other_mod) {
            // requested import module not found
            i_info_ctx_add_msg(
                m->front->info,
                MESSAGE_SEMANTIC_ERROR,
                ast_node_startmark(import),
                ast_node_endmark(import),
                "Requested module \""RFS_PF"\" not found for importing.",
                RFS_PA(ast_identifier_str(*c))
            );
            return false;
        }
        darray_append(m->dependencies, other_mod);
    }
    return true;
}

const struct RFstring *module_name(const struct module *m)
{
    RF_ASSERT(m->rir || m->node, "Expected either a rir or a roor node to exist");
    return module_rir_codepath(m) == RIRPOS_PARSE
        ? &m->rir->name
        : m->node->type == AST_ROOT ? &g_str_main : ast_module_name(m->node);
}

struct symbol_table *module_symbol_table(const struct module *m)
{
    return ast_node_symbol_table_get(m->node);
}

bool module_symbol_table_init(struct module *m)
{
    RF_ASSERT(m->node && (m->node->type == AST_ROOT || m->node->type == AST_MODULE),
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
    return rf_string_equal(module_name(m), &g_str_main);
}

enum rir_pos module_rir_codepath(const struct module *m)
{
    return m->front->parser->type == PARSER_AST ? RIRPOS_AST : RIRPOS_PARSE;
}

bool module_add_stdlib(struct module *m)
{
    struct module *other_mod = compiler_module_get(&g_str_stdlib);
    if (!other_mod) {
        RF_ERROR("stdlib was requested but could not be found in the parsed compiler modules");
        return false;
    }
    darray_append(m->dependencies, other_mod);
    return true;
}


bool module_types_set_add(struct module *m, struct type *new_type, const struct ast_node *n)
{
    (void)n;
    return rf_objset_add(m->types_set, type, new_type);
}

struct type *module_types_set_has_str(const struct module *m, const struct RFstring *s)
{
    return type_objset_has_string(m->types_set, s);
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
    if (module_rir_codepath(m) == RIRPOS_AST) {
        // determine dependencies if we are coming from AST parsing

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
    RF_ASSERT(!m->rir, "Should not come here from a RIR parsing codepath");
    bool ret = false;
    // since analyze pass is always going to be one per thread initializing
    // thread local type creation context here should be okay
    type_creation_ctx_init();
    // create symbol tables and change ast nodes ownership
    if (!analyzer_first_pass(m)) {
        if (!module_have_errors(m)) {
            RF_ERROR("Failure at module analysis first pass");
        }
        goto end;
    }

    if (!analyzer_typecheck(m, m->node)) {
        if (!module_have_errors(m)) {
            RF_ERROR("Failure at module's typechecking");
        }
        goto end;
    }

    if (!analyzer_finalize(m)) {
        RF_ERROR("Failure at module's finalization");
        goto end;
    }

    // success
    ret = true;
end:
    type_creation_ctx_deinit();
    return ret;
}

bool module_have_errors(const struct module *m)
{
    return info_ctx_has(m->front->info, MESSAGE_SEMANTIC_ERROR | MESSAGE_SYNTAX_ERROR);
}
