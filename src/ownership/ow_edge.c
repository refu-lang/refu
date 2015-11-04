#include "ow_edge.h"

struct ow_edge *ow_edge_create(const struct rir_expression *expr, const struct rir_value *nodeval)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init(&ret->to, nodeval);
    ret->edgeexpr = expr;
    return ret;
}

struct ow_edge *ow_endedge_create(const struct rir_expression *expr, enum ow_end_type end_type)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init_end(&ret->to, end_type);
    ret->edgeexpr = expr;
    return ret;
}

void ow_edge_destroy(struct ow_edge *e)
{
    ow_node_deinit(&e->to);
    free(e);
}
