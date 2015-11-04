#ifndef LFR_OWNERSHIP_GRAPH_H
#define LFR_OWNERSHIP_GRAPH_H

#include <Utils/bits.h>
#include <Definitions/inline.h>
#include "ow_node.h"

struct rir_expression;
struct rir_object;

//! Graph attributes
enum graph_attrs {
    OW_ATTR_RETURNED = 0x1,
    OW_ATTR_PASSED = 0x2,
};

struct ow_graph {
    const struct RFstring *fn_name;
    struct rir_expression *expr;
    struct ow_node *root;
    struct rf_objset_ownode set;
    int graph_attrs;
    struct symbol_table_record *rec;
};

struct ow_graph *ow_graph_create(struct rir_expression *expr,
                                 const struct RFstring *name,
                                 struct symbol_table_record *rec);
void ow_graph_destroy(struct ow_graph *g);

bool ow_graph_check_or_add_val(struct ow_graph *g,
                               const struct rir_value *v,
                               const struct rir_value *dependentv,
                               const struct rir_expression *edgexpr);

bool ow_graph_check_or_add_end(struct ow_graph *g,
                               const struct rir_value *v,
                               enum ow_end_type end_type,
                               const struct rir_expression *edgexpr);

void ow_graph_set_attr(struct ow_graph *g, enum graph_attrs attr);

i_INLINE_DECL bool ow_graph_has_attr(const struct ow_graph *g, enum graph_attrs attr)
{
    return RF_BITFLAG_ON(g->graph_attrs, attr);
}

#if RF_WITH_GRAPHVIZ
/**
 * Turn a graph to a graphviz dot graph
 */
bool ow_graph_to_graphviz(struct ow_graph *g);
#endif

#endif
