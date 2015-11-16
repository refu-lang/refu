#include "ow_edge.h"

static void ow_edge_init(struct ow_edge *e,
                         enum ow_edge_type type,
                         const struct rir_expression *expr,
                         struct ow_node *tonode)
{
    e->type = type;
    e->edgeexpr = expr;
    e->to = tonode;
}

struct ow_edge *ow_edge_create(const struct rir_expression *expr, const struct RFstring *name, const struct rir_value *nodeval)
{
    struct ow_edge *ret;
    struct ow_node *n;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!(n = ow_node_create(name, nodeval))) {
        free(ret);
        return NULL;
    }
    ow_edge_init(ret, OW_EDGE_NORMAL, expr, n);
    return ret;
}

struct ow_edge *ow_edge_create_from_node(const struct rir_expression *expr, struct ow_node *tonode)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_edge_init(ret, OW_EDGE_TO_EXISTING, expr, tonode);
    return ret;
}

struct ow_edge *ow_endedge_create(const struct rir_expression *expr, const struct RFstring *fnname, enum ow_end_type end_type)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    struct ow_node *n;
    if (!(n = ow_node_end_create(fnname, end_type))) {
        free(ret);
        return NULL;
    }
    ow_edge_init(ret, OW_EDGE_NORMAL, expr, n);
    return ret;
}

void ow_edge_destroy(struct ow_edge *e)
{
    if (e->type == OW_EDGE_NORMAL) {
        ow_node_destroy(e->to);
    }
    free(e);
}
