#include <module.h>

#include <compiler.h>
#include <front_ctx.h>
#include <ast/ast.h>
#include <ast/module.h>
#include <analyzer/analyzer.h>

static const struct RFstring g_main_module_str = RF_STRING_STATIC_INIT("main");

struct module *module_create(struct ast_node *n, struct front_ctx *front)
{
    struct module *ret;
    RF_ASSERT(n->type == AST_MODULE || n->type == AST_ROOT,
              "Unexpected ast node type");
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    // initialize
    ret->analyzer = NULL;
    ret->node = n;
    ret->front = front;
    darray_init(ret->dependencies);
    // add to the compiler's modules
    darray_append(compiler_instance_get()->modules, ret);
    return ret;
}

void module_destroy(struct module* m)
{
    darray_free(m->dependencies);
    if (m->analyzer) {
        analyzer_destroy(m->analyzer);
    }
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

struct inpfile *module_get_file(const struct module *m)
{
    return m->analyzer->info->file;
}

bool module_analyze(struct module *m)
{
    m->analyzer = analyzer_create(m->front->info);
    if (!m->analyzer) {
        return false;
    }
    return analyzer_analyze_module(m);
}
