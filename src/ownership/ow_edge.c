#include "ow_edge.h"
#include <ir/rir_expression.h>

static void ow_edge_init(struct ow_edge *e,
                         enum ow_edge_type type,
                         const struct rir_expression *expr,
                         struct ow_node *tonode,
                         unsigned int counter)
{
    e->type = type;
    e->edgeexpr = expr;
    e->to = tonode;
    e->counter = counter;
}

struct ow_edge *ow_edge_create(const struct rir_expression *expr, const struct RFstring *name, const struct rir_value *nodeval, unsigned int counter)
{
    struct ow_edge *ret;
    struct ow_node *n;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!(n = ow_node_create(name, nodeval))) {
        free(ret);
        return NULL;
    }
    ow_edge_init(ret, OW_EDGE_NORMAL, expr, n, counter);
    return ret;
}

struct ow_edge *ow_edge_create_from_node(const struct rir_expression *expr, struct ow_node *tonode, unsigned int counter)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_edge_init(ret, OW_EDGE_TO_EXISTING, expr, tonode, counter);
    return ret;
}

struct ow_edge *ow_endedge_create(const struct rir_expression *expr, const struct RFstring *fnname, enum ow_end_type end_type, unsigned int counter)
{
    struct ow_edge *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    struct ow_node *n;
    if (!(n = ow_node_end_create(fnname, end_type, expr->type == RIR_EXPRESSION_CALL ? &expr->call.name : NULL))) {
        free(ret);
        return NULL;
    }
    ow_edge_init(ret, OW_EDGE_NORMAL, expr, n, counter);
    return ret;
}

void ow_edge_destroy(struct ow_edge *e)
{
    if (e->type == OW_EDGE_NORMAL) {
        ow_node_destroy(e->to);
    }
    free(e);
}
