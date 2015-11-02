#include "ow_graph.h"
#include <ir/rir_object.h>

struct ow_graph *ow_graph_create(struct rir_value *v, struct rir_object *obj)
{
    struct ow_graph *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->val = v;
    ret->root = ow_node_create(obj);
    if (!ret->root) {
        return false;
    }
    rf_objset_init(&ret->set, ownode);
    rf_objset_add(&ret->set, ownode, ret->root);
    return ret;
}

void ow_graph_destroy(struct ow_graph *g)
{
    ow_node_destroy_recursive(g->root);
    rf_objset_clear(&g->set);
    free(g);
}

bool ow_graph_check_or_add_val(struct ow_graph *g, struct rir_object *obj)
{
    struct ow_node *n;
    struct rir_value *v = rir_object_value(obj);
    if (!v) {
        return false;
    }
    if (!(n = ownode_objset_has_value(&g->set, v))) {
        return false;
    }
    struct ow_node *new_node = ow_node_create(obj);
    if (!new_node) {
        return false;
    }
    ow_node_add(n, new_node);
    return true;
}
