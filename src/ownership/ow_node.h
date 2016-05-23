#ifndef LFR_OWNERSHIP_NODE_H
#define LFR_OWNERSHIP_NODE_H

#include <rfbase/datastructs/objset.h>
#include <rfbase/datastructs/darray.h>
#include <rfbase/defs/inline.h>
#include <rfbase/utils/sanity.h>

struct rir_value;
struct rir_expression;
struct ow_edge;

struct ow_node_full {
    const struct rir_value *val;
};

enum ow_end_type {
    OW_END_RETURN,
    OW_END_PASSED
};

struct ow_node_end {
    enum ow_end_type type;
    const struct RFstring *other_fn_name;
};

enum ow_node_type {
    OW_NTYPE_FULL,
    OW_NTYPE_END,
};
struct ow_node {
    enum ow_node_type type;
    const struct RFstring *fnname;
    struct {darray(struct ow_edge*);} edges;
    union {
        struct ow_node_full full;
        struct ow_node_end end;
    };
};

void ow_node_init(struct ow_node *n, const struct rir_value *nodeval);
struct ow_node *ow_node_create(const struct RFstring *fnname, const struct rir_value *nodeval);
struct ow_node *ow_node_end_create(const struct RFstring *fnname, enum ow_end_type end_type, const struct RFstring *other_fn_name);

bool ow_node_connect_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other);
bool ow_node_connect_end_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other);

void ow_node_deinit(struct ow_node *n);
void ow_node_destroy(struct ow_node *n);

/**
 * Given a unique ID for this ow node graph
 * @warning Should be wrapped in RFS_PUSH() / RFS_POP()
 */
const struct RFstring *ow_node_id(const struct ow_node *n);

struct ow_node *ow_node_add_val_edge(struct ow_node *n, const struct rir_value *otherval, const struct rir_expression *expr);
struct ow_node *ow_node_add_end_edge(struct ow_node *n, enum ow_end_type end_type, const struct rir_expression *expr);

const struct RFstring *ow_node_end_type_str(enum ow_end_type type);

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

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, const struct RFstring *fnname, const struct rir_value *v);

#endif
