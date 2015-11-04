#include "ow_graph.h"
#include <ir/rir_object.h>

/* #define DD(...) printf(__VA_ARGS__) */
#define DD(...)

struct ow_graph *ow_graph_create(struct rir_expression *expr,
                                 const struct RFstring *name,
                                 struct symbol_table_record *rec)
{
    struct ow_graph *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->fn_name = name;
    ret->expr = expr;
    ret->root = ow_node_create(&expr->val);
    ret->graph_attrs = 0;
    ret->rec = rec;
    if (!ret->root) {
        return false;
    }
    rf_objset_init(&ret->set, ownode);
    DD("Creating graph for value "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(rir_value_string(&expr->val)));
    rf_objset_add(&ret->set, ownode, ret->root);
    return ret;
}

void ow_graph_destroy(struct ow_graph *g)
{
    ow_node_destroy(g->root);
    rf_objset_clear(&g->set);
    free(g);
}

bool ow_graph_check_or_add_val(struct ow_graph *g,
                               const struct rir_value *v,
                               const struct rir_value *dependentv,
                               const struct rir_expression *edgexpr)
{
    struct ow_node *n;
    DD("Checking if value "RF_STR_PF_FMT" can go to graph for value "RF_STR_PF_FMT"\n",
           RF_STR_PF_ARG(rir_value_string(v)),
           RF_STR_PF_ARG(rir_value_string(&g->expr->val))
    );
    if (!(n = ownode_objset_has_value(&g->set, v))) {
        DD("No it can't!\n");
        return false;
    }
    DD("Yes it can.\n");
    if (!(n = ow_node_add_val_edge(n, dependentv, edgexpr))) {
        DD("Failed to add a new node as an edge");
        return false;
    }
    rf_objset_add(&g->set, ownode, n);
    return true;
}

bool ow_graph_check_or_add_end(struct ow_graph *g,
                               const struct rir_value *v,
                               enum ow_end_type end_type,
                               const struct rir_expression *edgexpr)
{
    struct ow_node *n;
    DD("Checking if value "RF_STR_PF_FMT" can go to graph for value "RF_STR_PF_FMT"\n",
           RF_STR_PF_ARG(rir_value_string(v)),
           RF_STR_PF_ARG(rir_value_string(&g->expr->val))
    );
    if (!(n = ownode_objset_has_value(&g->set, v))) {
        DD("No it can't!\n");
        return false;
    }
    DD("Yes it can.\n");
    if (!(n = ow_node_add_end_edge(n, end_type, edgexpr))) {
        DD("Failed to add a new node as an edge");
        return false;
    }
    switch (end_type) {
    case OW_END_RETURN:
        ow_graph_set_attr(g, OW_ATTR_RETURNED);
        break;
    }
    return true;
}

void ow_graph_set_attr(struct ow_graph *g, enum graph_attrs attr)
{
    RF_BITFLAG_SET(g->graph_attrs, attr);
    RF_ASSERT(g->expr->type == RIR_EXPRESSION_ALLOCA, "An alloca should exist here");
    // since the graph states the value is returned, allocation needs to be in the heap
    g->expr->alloca.alloc_location = RIR_ALLOC_HEAP;
}

i_INLINE_INS bool ow_graph_has_attr(const struct ow_graph *g, enum graph_attrs attr);
