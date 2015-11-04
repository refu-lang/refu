#include <ownership/ownership.h>
#include <compiler.h>
#include <ir/rir.h>
#include <ir/rir_function.h>
#include <ir/rir_block.h>
#include <analyzer/symbol_table.h>
#include <Data_Structures/objset.h>

#include "ow_graph.h"
#if RF_WITH_GRAPHVIZ
#include "ow_graphviz.h"
#endif

struct objset_st { OBJSET_MEMBERS(struct symbol_table*); };


struct ow_ctx {
    struct {darray(struct ow_graph*);} graphs;
};
i_THREAD__ struct ow_ctx *g_ow_ctx = NULL;

static void ow_ctx_init()
{
    RF_ASSERT_OR_EXIT(!g_ow_ctx, "Global Ownership context was already initialized.");
    g_ow_ctx = malloc(sizeof(struct ow_ctx));
    RF_ASSERT_OR_EXIT(g_ow_ctx, "Could not allocate a global ownership context");
    darray_init(g_ow_ctx->graphs);
}

static void ow_ctx_reset()
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    struct ow_graph **g;
    darray_foreach(g, g_ow_ctx->graphs) {
        ow_graph_destroy(*g);
    }
    darray_clear(g_ow_ctx->graphs);
}

static void ow_ctx_deinit()
{
    ow_ctx_reset();
    darray_free(g_ow_ctx->graphs);
    free(g_ow_ctx);
    g_ow_ctx = NULL;
}

static inline bool ow_ctx_add_new(struct rir_expression *expr, const struct RFstring *fn_name, struct symbol_table *st)
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    // find the symbol table record for this rir object
    struct symbol_table_record *rec = symbol_table_lookup_rirobj(st, rir_expression_to_obj(expr));
    if (!rec) {
        // at least for now, if no record was discovered we are not interested in
        // creating a graph for this alloca
        return true;
    }
    struct ow_graph *g = ow_graph_create(expr, fn_name, rec);
    if (!g) {
        return false;
    }
    darray_append(g_ow_ctx->graphs, g);
    return true;
}

#if RF_WITH_GRAPHVIZ
bool ow_ctx_create_graphviz()
{
    bool ret = false;
    if (!ow_graphviz_init()) {
        RF_ERROR("Failed to initialize graphviz data");
        return false;
    }
    struct ow_graph **graph;
    darray_foreach(graph, g_ow_ctx->graphs) {
        if (!ow_graph_to_graphviz(*graph)) {
            goto end;
        }
    }
    //success
    ret = true;
end:
    ow_graphviz_deinit();
    return ret;
}
#endif

/**
 * Check if a value should go to a graph
 *
 * @param v              The value for which to check for attachment to a graph
 * @param dv             The value to put into the graph as a dependency
 * @param expr           The rir expression that should go at the connecting edge
 */
static void ow_ctx_check_value(const struct rir_value *v, const struct rir_value *dv, struct rir_expression *expr)
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    struct ow_graph **graph;
    darray_foreach(graph, g_ow_ctx->graphs) {
        if (ow_graph_check_or_add_val(*graph, v, dv, expr)) {
            return; // stop searching if it gets added to any graph
        }
    }
}

/**
 * Acts just like @ref ow_ctx_check_value() but considers the expression's value
 * to be the value to add in the graph as a dependency
 */
static inline void ow_ctx_check_value_from_expr(const struct rir_value *v, struct rir_expression *expr)
{
    ow_ctx_check_value(v, &expr->val, expr);
}

/**
 * Check if a value should go to a graph as an end node
 *
 * @param v              The value for which to check for attachment to a graph
 * @param end_type       The end node type
 * @param expr           The rir expression that should go at the connecting edge
 */
static void ow_ctx_check_value_as_end(const struct rir_value *v, enum ow_end_type end_type, const struct rir_expression *expr)
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    struct ow_graph **graph;
    darray_foreach(graph, g_ow_ctx->graphs) {
        if (ow_graph_check_or_add_end(*graph, v, end_type, expr)) {
            return; // stop searching if it gets added to any graph
        }
    }
}

void ow_ctx_check_expr(struct rir_expression *expr)
{
    switch(expr->type) {
    case RIR_EXPRESSION_CALL:
        // check if any of the call's arguments should be in the graph
    {
        struct rir_value **val;
        darray_foreach(val, expr->call.args) {
            ow_ctx_check_value_from_expr(*val, expr);
        }
    }
        break;
    case RIR_EXPRESSION_OBJMEMBERAT:
        ow_ctx_check_value_from_expr(expr->objmemberat.objmemory, expr);
        break;
    case RIR_EXPRESSION_UNIONMEMBERAT:
        ow_ctx_check_value_from_expr(expr->unionmemberat.unimemory, expr);
        break;
    case RIR_EXPRESSION_READ:
        ow_ctx_check_value_from_expr(expr->read.memory, expr);
        break;
    case RIR_EXPRESSION_WRITE:
        ow_ctx_check_value(expr->write.writeval, expr->write.memory, expr);
        break;
    case RIR_EXPRESSION_CONVERT:
        ow_ctx_check_value_from_expr(expr->convert.val, expr);
        break;
    case RIR_EXPRESSION_ADD:
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
        ow_ctx_check_value_from_expr(expr->binaryop.a, expr);
        ow_ctx_check_value_from_expr(expr->binaryop.b, expr);
        break;
    case RIR_EXPRESSION_RETURN:
        // returns are handled at the end of each block's iteration (block exits)
    case RIR_EXPRESSION_SETUNIONIDX:
    case RIR_EXPRESSION_GETUNIONIDX:
    case RIR_EXPRESSION_CONSTANT:

    case RIR_EXPRESSION_CMP_EQ:
    case RIR_EXPRESSION_CMP_NE:
    case RIR_EXPRESSION_CMP_GE:
    case RIR_EXPRESSION_CMP_GT:
    case RIR_EXPRESSION_CMP_LE:
    case RIR_EXPRESSION_CMP_LT:

    case RIR_EXPRESSION_LOGIC_AND:
    case RIR_EXPRESSION_LOGIC_OR:
    case RIR_EXPRESSION_PLACEHOLDER:
        // no need to do anything
        break;
    default:
        RF_CRITICAL_FAIL("Unexpected expression type. Should never happen");
        break;
    }
}

bool ow_function_pass(struct rir_fndef *f)
{
    struct objset_st set;
    rf_objset_init_default(&set);
    struct rir_block **b;
    darray_foreach(b, f->blocks) {
        if (!rf_objset_get_default(&set, (*b)->st)) {
            /* symbol_table_print((*b)->st); */
            rf_objset_add_default(&set, (*b)->st);
        }

        struct rir_expression *expr;
        rf_ilist_for_each(&(*b)->expressions, expr, ln) {
            // if it's an alloca add a new graph
            if (expr->type == RIR_EXPRESSION_ALLOCA) {
                if (!ow_ctx_add_new(expr, f->decl.name, (*b)->st)) {
                    RF_ERROR("Error at adding a new ownership graph");
                    return false;
                }
            } else { // in other cases see if it needs to go in a graph
                ow_ctx_check_expr(expr);
            }
        }
        // also check if this is the return block
        struct rir_expression *rexp = &(*b)->exit.retstmt;
        if ((*b)->exit.type == RIR_BLOCK_EXIT_RETURN && rexp->ret.val) {
            ow_ctx_check_value_as_end(&rexp->ret.val->val, OW_END_RETURN, rexp->ret.val);
        }
    }
    rf_objset_clear(&set);
    return true;
}

bool ow_module_pass(struct rir *r)
{
    // output functions
    struct rir_fndecl *decl;
    rf_ilist_for_each(&r->functions, decl, ln) {
        ow_ctx_reset(); // for now the graph is per function
        if (!decl->plain_decl && !ow_function_pass(rir_fndecl_to_fndef(decl))) {
            return false;
        }
#if RF_WITH_GRAPHVIZ
        // for now just always create the SVG graphs per function
        ow_ctx_create_graphviz();
#endif
    }
    return true;
}

bool ownership_pass(struct compiler *c)
{
    struct module **mod;
    bool ret = false;
    ow_ctx_init();
    darray_foreach(mod, c->modules) {
        if (!ow_module_pass((*mod)->rir)) {
            goto end;
        }
    }
    // success
    ret = true;

end:
    ow_ctx_deinit();
    return ret;
}
