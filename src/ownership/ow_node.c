#include "ow_node.h"
#include "ow_edge.h"
#include <ir/rir_object.h>
#include <String/rf_str_core.h>

/* static const struct ow_node g_return_node = { .type = OW_NTYPE_END, {.end_type = OW_END_RETURN}}; */

void ow_node_init(struct ow_node *n, const struct rir_value *nodeval)
{
    n->type = OW_NTYPE_FULL;
    darray_init(n->full.edges);
    n->full.val = nodeval;
}

void ow_node_init_end(struct ow_node *n, enum ow_end_type end_type)
{
    n->type = OW_NTYPE_END;
    n->end_type = end_type;
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
    if (n->type == OW_NTYPE_END) {
        return;
    }
    struct ow_edge **e;
    darray_foreach(e, n->full.edges) {
        ow_edge_destroy(*e);
    }
    darray_free(n->full.edges);
}


void ow_node_destroy(struct ow_node *n)
{
    if (n->type == OW_NTYPE_FULL) {
        ow_node_deinit(n);
        free(n);
    }
}

struct ow_node *ow_node_add_val_edge(struct ow_node *n, const struct rir_value *otherval, const struct rir_expression *expr)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    struct ow_edge *e = ow_edge_create(expr, otherval);
    if (!e) {
        return NULL;
    }
    darray_append(n->full.edges, e);
    return &e->to;
}

struct ow_node *ow_node_add_end_edge(struct ow_node *n, enum ow_end_type end_type, const struct rir_expression *expr)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    struct ow_edge *e = ow_endedge_create(expr, end_type);
    if (!e) {
        return NULL;
    }
    darray_append(n->full.edges, e);
    return &e->to;
}

static const struct RFstring ow_endnode_strings[] = {
    [OW_END_RETURN] = RF_STRING_STATIC_INIT("return")
};

const struct RFstring *ow_node_end_type_str(enum ow_end_type type)
{
    return &ow_endnode_strings[type];
}



i_INLINE_INS const void *ownode_objset_key(const struct ow_node *n);

size_t ownode_objset_hashfn(const struct ow_node *n)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    return rf_hash_str_stable(&n->full.val->id, 0);
}

bool ownode_objset_eqfn(const struct ow_node *n1,
                        const struct ow_node *n2)
{
    RF_ASSERT(n1->type != OW_NTYPE_END, "No end node should appear here");
    RF_ASSERT(n2->type != OW_NTYPE_END, "No end node should appear here");
    return rf_string_equal(&n1->full.val->id,
                           &n2->full.val->id);
}

static bool id_cmp_fn(struct ow_node *n, size_t *id)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    return rf_hash_str_stable(&n->full.val->id, 0) == *id;
}

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, const struct rir_value *v)
{
    size_t id = rf_hash_str_stable(&v->id, 0);
    return htable_get(&set->raw.ht,
                      id,
                      (bool (*)(const void *, void *))id_cmp_fn,
                      &id);
}
