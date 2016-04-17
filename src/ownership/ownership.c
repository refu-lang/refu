#include <ownership/ownership.h>

#include <rflib/datastructs/objset.h>

#include <compiler.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <ir/rir_block.h>
#include <ir/rir_call.h>
#include <analyzer/symbol_table.h>

#include "ow_graph.h"
#include "ow_debug.h"
#if RF_WITH_GRAPHVIZ
#include "ow_graphviz.h"
#endif

struct objset_st { OBJSET_MEMBERS(struct symbol_table*); };


struct ow_ctx {
    struct {darray(struct ow_graph*);} graphs;
    const struct RFstring *curr_fn_name;
    unsigned int rir_expr_idx;
};
i_THREAD__ struct ow_ctx *g_ow_ctx = NULL;

const struct RFstring *ow_curr_fnname()
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    return g_ow_ctx->curr_fn_name;
}

void ow_reset_expr_idx()
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    g_ow_ctx->rir_expr_idx = 0;
}

unsigned int ow_expr_idx_inc()
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    return g_ow_ctx->rir_expr_idx++;
}

unsigned int ow_expr_idx()
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    return g_ow_ctx->rir_expr_idx;
}

static void ow_ctx_init()
{
    RF_ASSERT_OR_EXIT(!g_ow_ctx, "Global Ownership context was already initialized.");
    g_ow_ctx = malloc(sizeof(struct ow_ctx));
    RF_ASSERT_OR_EXIT(g_ow_ctx, "Could not allocate a global ownership context");
    RF_STRUCT_ZERO(g_ow_ctx);
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

static inline bool ow_ctx_add_new(struct rir_object *obj, const struct RFstring *fn_name, struct symbol_table *st)
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    // find the symbol table record for this rir object
    struct symbol_table_record *rec = symbol_table_lookup_rirobj(st, obj);
    if (!rec) {
        // at least for now, if no record was discovered we are not interested in
        // creating a graph for this alloca
        return true;
    }
    struct ow_graph *g = ow_graph_create(obj, fn_name, rec);
    if (!g) {
        return false;
    }
    darray_append(g_ow_ctx->graphs, g);
    return true;
}

static struct ow_graph *ow_ctx_graph_from_ploc(struct ow_passed_loc *ploc, struct ow_graph *from_graph)
{
    RF_ASSERT(!ploc->call->foreign, "Foreign function calls should not come here");
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    struct ow_graph **g;
    OWDD("Trying to find ploc \""RFS_PF"\" in the following graphs:\n", RFS_PA(&ploc->call->name)); 
    darray_foreach(g, g_ow_ctx->graphs) {
        OWDD("Graph "RFS_PF"\n", RFS_PA(ow_node_id((*g)->root)));
    }
    darray_foreach(g, g_ow_ctx->graphs) {
        RFS_PUSH();
        if (rf_string_equal(&ploc->call->name, (*g)->fn_name) &&
            (*g)->obj->category == RIR_OBJ_VARIABLE &&
            rf_string_equal(rir_value_string(rir_object_value((*g)->obj)),
                            RFS("$%u", ploc->idx))
        ) {
            RFS_POP();
            // change ploc node to the actual node it should point to
            ploc->node = (*g)->root;
            return *g;
        }
        RFS_POP();
    }
    // else fail
    return NULL;
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
 * @param idx            If this concerns parameter passing this is the parameter index. Else zero.
 */
static void ow_ctx_check_value_as_end(const struct rir_value *v,
                                      enum ow_end_type end_type,
                                      const struct rir_expression *expr,
                                      unsigned int idx)
{
    RF_ASSERT(g_ow_ctx, "No global ownership context exists");
    struct ow_graph **graph;
    darray_foreach(graph, g_ow_ctx->graphs) {
        if (ow_graph_check_or_add_end(*graph, v, end_type, expr, idx)) {
            return; // stop searching if it gets added to any graph
        }
    }
}

void ow_ctx_check_expr(struct rir_expression *expr)
{
    OWDD("\n>>> Checking a new expression\n");
    switch(expr->type) {
    case RIR_EXPRESSION_CALL:
        // check if any of the call's arguments should be in the graph
    {
        struct rir_value **val;
        unsigned int idx = 0;
        darray_foreach(val, expr->call.args) {
            ow_ctx_check_value_from_expr(*val, expr);
            ow_ctx_check_value_as_end(*val, OW_END_PASSED, expr, idx++);
        }
    }
        break;
    case RIR_EXPRESSION_FIXEDARR:
        // check if any of the array's members should be in the graph
    {
        struct rir_value **val;
        darray_foreach(val, expr->fixedarr.members) {
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
    struct rir_object **var;
    ow_reset_expr_idx();
    g_ow_ctx->curr_fn_name = &f->decl.name;
    darray_foreach(var, f->variables) {
        if (!ow_ctx_add_new(*var, &f->decl.name, f->st)) {
            RF_ERROR("Error at adding a new ownership graph");
            return false;
        }
    }

    struct rir_block **b;
    darray_foreach(b, f->blocks) {
        struct rir_expression *expr;
        rf_ilist_for_each(&(*b)->expressions, expr, ln) {
            // if it's an alloca add a new graph
            if (expr->type == RIR_EXPRESSION_ALLOCA) {
                if (!ow_ctx_add_new(rir_expression_to_obj(expr), &f->decl.name, (*b)->st)) {
                    RF_ERROR("Error at adding a new ownership graph");
                    return false;
                }
            } else { // in other cases see if it needs to go in a graph
                ow_ctx_check_expr(expr);
            }
        }
        // also check if this is the return block
        struct rir_return *rexp = &(*b)->exit.retstmt;
        if ((*b)->exit.type == RIR_BLOCK_EXIT_RETURN && rexp->val) {
            ow_ctx_check_value_as_end(rexp->val, OW_END_RETURN, NULL, 0);
        }
    }
    return true;
}

bool ow_module_pass(struct rir *r)
{
    // output functions
    struct rir_fndecl *decl;
    rf_ilist_for_each(&r->functions, decl, ln) {
        if (!decl->plain_decl && !ow_function_pass(rir_fndecl_to_fndef(decl))) {
            return false;
        }
    }

    struct ow_graph **graph;
    // Now search all the graphs and see which are connecting and connect them
    darray_foreach(graph, g_ow_ctx->graphs) {
        struct ow_passed_loc **ploc;
        darray_foreach(ploc, (*graph)->passed_locations) {
            if ((*ploc)->call->foreign) {
                continue;
            }
            // find the graph of the passed function
            struct ow_graph *pgraph = ow_ctx_graph_from_ploc(*ploc, *graph);
            if (!pgraph) {
                RF_ERROR("Failed to find the graph from a passed location");
                return false;
            }
            // connect them
            /* if (!ow_node_connect_end_node((*graph)->root, rir_call_to_expr((*ploc)->call), (*ploc)->node)) { */
            if (!ow_node_connect_end_node((*ploc)->from_node, rir_call_to_expr((*ploc)->call), (*ploc)->node)) {
                RF_ERROR("Failed to connect nodes of two different graphs");
                return false;
            }
        }
    }
    
#if RF_WITH_GRAPHVIZ
    rf_ilist_for_each(&r->functions, decl, ln) {
        // for now just always create the SVG graphs per function
        ow_ctx_create_graphviz();
    }
#endif
    return true;
}

bool ownership_pass(struct compiler *c)
{
    struct module **mod;
    bool ret = false;
    ow_ctx_init();
    darray_foreach(mod, c->modules) {
        // only for modules that got parsed from normal source, at least for now
        if (module_rir_codepath(*mod) == RIRPOS_AST) {
            ow_ctx_reset(); // for now the graph is per module
            if (!ow_module_pass((*mod)->rir)) {
                goto end;
            }
        }
    }
    // success
    ret = true;

end:
    ow_ctx_deinit();
    return ret;
}
