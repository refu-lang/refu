#ifndef LFR_OWNERSHIP_NODE_H
#define LFR_OWNERSHIP_NODE_H

#include <Data_Structures/objset.h>
#include <Data_Structures/darray.h>
#include <Definitions/inline.h>
#include <Utils/sanity.h>

struct rir_value;

struct ow_node {
    struct rir_object *obj;
    struct {darray(struct ow_node*);} next_nodes;
};

void ow_node_init(struct ow_node *n, struct rir_object *obj);
struct ow_node *ow_node_create(struct rir_object *obj);

void ow_node_destroy(struct ow_node *n);
void ow_node_destroy_recursive(struct ow_node *n);

void ow_node_add(struct ow_node *n, struct ow_node *other);




i_INLINE_DECL const void *ownode_objset_key(const struct ow_node *n)
{
    return (const void*)n;
}

size_t ownode_objset_hashfn(const struct ow_node *n);
bool ownode_objset_eqfn(const struct ow_node *n1,
                        const struct ow_node *n2);

OBJSET_DEFINE_TYPE(ownode,
                   struct ow_node,
                   ownode_objset_key,
                   ownode_objset_hashfn,
                   ownode_objset_eqfn)

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, struct rir_value *v);

#endif
