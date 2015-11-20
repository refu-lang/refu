#ifndef LFR_OWNERSHIP_GRAPH_H
#define LFR_OWNERSHIP_GRAPH_H

#include <Utils/bits.h>
#include <Definitions/inline.h>
#include "ow_node.h"

struct rir_expression;
struct rir_object;

//! Graph attributes
enum graph_attrs {
    OW_ATTR_RETURNED = 1,
    OW_ATTR_PASSED = 2,
};

struct ow_passed_loc {
    const struct rir_call *call;
    struct ow_node *from_node;
    struct ow_node *node;
    unsigned int idx;
};

struct ow_passed_loc *ow_passed_loc_create(const struct rir_call *c,
                                           struct ow_node *n,
                                           unsigned int idx);
void ow_passed_loc_destroy(struct ow_passed_loc *ploc);

struct ow_graph {
    //! Name of the function this graph's value starts from
    const struct RFstring *fn_name;
    //! Rir object describing what this graph is for
    struct rir_object *obj;
    //! The root node of this graph
    struct ow_node *root;
    //! Set of ownership nodes that are related to the root node of the graph.
    //! Used to find if a node has a relation
    struct rf_objset_ownode set;
    //! Graph attributes denoting what happens to the value
    int graph_attrs;
    //! If the graph is marked as passing the value somewhere else these are the
    //! locations to connect to
    struct {darray(struct ow_passed_loc*);} passed_locations;
    //! The symbol table record of the rir object the graph is for
    struct symbol_table_record *rec;
};

struct ow_graph *ow_graph_create(struct rir_object *expr,
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
                               const struct rir_expression *edgexpr,
                               unsigned int idx);

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
