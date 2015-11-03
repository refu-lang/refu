#ifndef LFR_OWNERSHIP_EDGE_H
#define LFR_OWNERSHIP_EDGE_H

#include "ow_node.h"

struct rir_expression;
struct ow_edge {
    struct rir_expression *edgeexpr;
    struct ow_node to;
};

/**
 * Create an edge to a new node by specifying a value to create a node for
 */
struct ow_edge *ow_edge_create(struct rir_expression *expr, const struct rir_value *nodev);
void ow_edge_destroy(struct ow_edge *e);
#endif
