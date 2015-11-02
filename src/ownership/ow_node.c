#include "ow_node.h"
#include <ir/rir_object.h>
#include <String/rf_str_core.h>

void ow_node_init(struct ow_node *n, struct rir_object *obj)
{
    darray_init(n->next_nodes);
    n->obj = obj;
}

struct ow_node *ow_node_create(struct rir_object *obj)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init(ret, obj);
    return ret;
}

void ow_node_destroy(struct ow_node *n)
{
    darray_free(n->next_nodes);
    free(n);
}

void ow_node_destroy_recursive(struct ow_node *n)
{
    struct ow_node **nxt;
    darray_foreach(nxt, n->next_nodes) {
        ow_node_destroy_recursive(*nxt);
    }
    ow_node_destroy(n);
}

void ow_node_add(struct ow_node *n, struct ow_node *other)
{
    darray_append(n->next_nodes, other);
}



i_INLINE_INS const void *ownode_objset_key(const struct ow_node *n);

size_t ownode_objset_hashfn(const struct ow_node *n)
{
    return rf_hash_str_stable(&rir_object_value(n->obj)->id, 0);
}

bool ownode_objset_eqfn(const struct ow_node *n1,
                        const struct ow_node *n2)
{
    return rf_string_equal(&rir_object_value(n1->obj)->id,
                           &rir_object_value(n2->obj)->id);
}

static bool id_cmp_fn(struct ow_node *n, size_t *id)
{
    return rf_hash_str_stable(&rir_object_value(n->obj)->id, 0) == *id;
}

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, struct rir_value *v)
{
    size_t id = rf_hash_str_stable(&v->id, 0);
    return htable_get(&set->raw.ht,
                      id,
                      (bool (*)(const void *, void *))id_cmp_fn,
                      &id);
}
