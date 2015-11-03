#include "ow_node.h"
#include "ow_edge.h"
#include <ir/rir_object.h>
#include <String/rf_str_core.h>

void ow_node_init(struct ow_node *n, const struct rir_value *nodeval)
{
    darray_init(n->edges);
    n->val = nodeval;
}

struct ow_node *ow_node_create(const struct rir_value *nodeval)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init(ret, nodeval);
    return ret;
}

void ow_node_deinit(struct ow_node *n)
{
    struct ow_edge **e;
    darray_foreach(e, n->edges) {
        ow_edge_destroy(*e);
    }
    darray_free(n->edges);
}


void ow_node_destroy(struct ow_node *n)
{
    ow_node_deinit(n);
    free(n);
}

struct ow_node *ow_node_add_val_edge(struct ow_node *n, const struct rir_value *otherval, struct rir_expression *expr)
{
    struct ow_edge *e = ow_edge_create(expr, otherval);
    if (!e) {
        return NULL;
    }
    darray_append(n->edges, e);
    return &e->to;
}



i_INLINE_INS const void *ownode_objset_key(const struct ow_node *n);

size_t ownode_objset_hashfn(const struct ow_node *n)
{
    return rf_hash_str_stable(&n->val->id, 0);
}

bool ownode_objset_eqfn(const struct ow_node *n1,
                        const struct ow_node *n2)
{
    return rf_string_equal(&n1->val->id,
                           &n2->val->id);
}

static bool id_cmp_fn(struct ow_node *n, size_t *id)
{
    return rf_hash_str_stable(&n->val->id, 0) == *id;
}

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, const struct rir_value *v)
{
    size_t id = rf_hash_str_stable(&v->id, 0);
    return htable_get(&set->raw.ht,
                      id,
                      (bool (*)(const void *, void *))id_cmp_fn,
                      &id);
}
