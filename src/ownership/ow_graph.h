#ifndef LFR_OWNERSHIP_GRAPH_H
#define LFR_OWNERSHIP_GRAPH_H

#include "ow_node.h"
struct rir_expression;
struct rir_object;

struct ow_graph {
    const struct RFstring *fn_name;
    struct rir_value *val;
    struct ow_node *root;
    struct rf_objset_ownode set;
};

struct ow_graph *ow_graph_create(struct rir_value *v, const struct RFstring *name);
void ow_graph_destroy(struct ow_graph *g);

bool ow_graph_check_or_add_val(struct ow_graph *g,
                               const struct rir_value *v,
                               const struct rir_value *dependentv,
                               struct rir_expression *edgexpr);

#if RF_HAVE_GRAPHVIZ
/**
 * Turn a graph to a graphviz dot graph
 */
bool ow_graph_to_graphviz(struct ow_graph *g);
#endif

#endif
