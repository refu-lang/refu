#ifndef LFR_OWNERSHIP_EDGE_H
#define LFR_OWNERSHIP_EDGE_H

#include "ow_node.h"

struct rir_expression;
struct ow_edge {
    const struct rir_expression *edgeexpr;
    struct ow_node to;
};

/**
 * Create an edge to a new node by specifying a value to create a node for
 */
struct ow_edge *ow_edge_create(const struct rir_expression *expr, const struct rir_value *nodev);
/**
 * Create an edge to an end node
 */
struct ow_edge *ow_endedge_create(const struct rir_expression *expr, enum ow_end_type end_type);
void ow_edge_destroy(struct ow_edge *e);
#endif
