#ifndef LFR_OWNERSHIP_EDGE_H
#define LFR_OWNERSHIP_EDGE_H

#include "ow_node.h"

enum ow_edge_type {
    OW_EDGE_NORMAL,
    OW_EDGE_TO_EXISTING,
};

struct rir_expression;
struct ow_edge {
    enum ow_edge_type type;
    //! Can be NULL. Example: for returns
    const struct rir_expression *edgeexpr;
    struct ow_node *to;
    unsigned int counter;
};

/**
 * Create an edge to a new node by specifying a value to create a node for
 */
struct ow_edge *ow_edge_create(const struct rir_expression *expr, const struct RFstring *name, const struct rir_value *nodev, unsigned int counter);
/**
 * Create an edge to an end node
 */
struct ow_edge *ow_endedge_create(const struct rir_expression *expr, const struct RFstring *fnname, enum ow_end_type end_type, unsigned int counter);
/**
 * Create an edge without allocating a new node but just pointing to it
 */
struct ow_edge *ow_edge_create_from_node(const struct rir_expression *expr, struct ow_node *tonode, unsigned int counter);

void ow_edge_destroy(struct ow_edge *e);
#endif
