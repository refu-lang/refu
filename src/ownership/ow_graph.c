#include "ow_graph.h"
#include <ir/rir_object.h>

struct ow_graph *ow_graph_create(struct rir_value *v, const struct RFstring *name)
{
    struct ow_graph *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->fn_name = name;
    ret->val = v;
    ret->root = ow_node_create(v);
    if (!ret->root) {
        return false;
    }
    rf_objset_init(&ret->set, ownode);
    printf("Creating graph for value "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(rir_value_string(v)));
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
                               struct rir_expression *edgexpr)
{
    struct ow_node *n;
    printf("Checking if value "RF_STR_PF_FMT" can go to graph for value "RF_STR_PF_FMT"\n",
           RF_STR_PF_ARG(rir_value_string(v)),
           RF_STR_PF_ARG(rir_value_string(g->val))
    );
    if (!(n = ownode_objset_has_value(&g->set, v))) {
        printf("No it can't!\n");
        return false;
    }
    printf("Yes it can.\n");
    if (!(n = ow_node_add_val_edge(n, dependentv, edgexpr))) {
        printf("Failed to add a new node as an edge");
        return false;
    }
    rf_objset_add(&g->set, ownode, n);
    return true;
}
