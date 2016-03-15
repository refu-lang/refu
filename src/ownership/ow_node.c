#include "ow_node.h"

#include <rflib/string/rf_str_core.h>

#include "ow_edge.h"
#include "ow_debug.h"
#include <ownership/ownership.h>
#include <ir/rir_object.h>

static void ow_node_init_common(struct ow_node *n, const struct RFstring *fnname)
{
    darray_init(n->edges);
    n->fnname = fnname;
}

void ow_node_init(struct ow_node *n, const struct rir_value *nodeval)
{
    n->type = OW_NTYPE_FULL;
    n->full.val = nodeval;
}

static void ow_node_init_end(struct ow_node *n, enum ow_end_type end_type, const struct RFstring *other_fn_name)
{
    n->type = OW_NTYPE_END;
    n->end.type = end_type;
    n->end.other_fn_name = other_fn_name;
}

struct ow_node *ow_node_create(const struct RFstring *fnname, const struct rir_value *nodeval)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init_common(ret, fnname);
    ow_node_init(ret, nodeval);
    return ret;
}

struct ow_node *ow_node_end_create(const struct RFstring *fnname, enum ow_end_type end_type, const struct RFstring *other_fn_name)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ow_node_init_common(ret, fnname);
    ow_node_init_end(ret, end_type, other_fn_name);
    return ret;
}

bool ow_node_connect_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other)
{
    struct ow_edge *e = ow_edge_create_from_node(expr, other, ow_expr_idx_inc());
    if (!e) {
        return false;
    }
    darray_append(n->edges, e);
    return true;
}

bool ow_node_connect_end_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other)
{
    OWDD(
        "Connect node \""RFS_PF"\" with \""RFS_PF"\" with counter: %u\n",
        RFS_PA(ow_node_id(n)),
        RFS_PA(ow_node_id(other)),
        ow_expr_idx()
    );
    return ow_node_connect_node(n, expr, other);
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

struct ow_node *ow_node_add_val_edge(struct ow_node *n, const struct rir_value *otherval, const struct rir_expression *expr)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    OWDD(
        "Create edge from  node \""RFS_PF"\" with counter: %u\n",
        RFS_PA(ow_node_id(n)),
        ow_expr_idx()
    );
    struct ow_edge *e = ow_edge_create(expr, n->fnname, otherval, ow_expr_idx_inc());
    if (!e) {
        return NULL;
    }
    darray_append(n->edges, e);
    return e->to;
}

struct ow_node *ow_node_add_end_edge(struct ow_node *n, enum ow_end_type end_type, const struct rir_expression *expr)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    OWDD(
        "Create edge from  node \""RFS_PF"\" with counter: %u\n",
        RFS_PA(ow_node_id(n)),
        ow_expr_idx()
    );
    struct ow_edge *e = ow_endedge_create(expr, n->fnname, end_type, ow_expr_idx_inc());
    if (!e) {
        return NULL;
    }
    darray_append(n->edges, e);
    return e->to;
}

static const struct RFstring ow_endnode_strings[] = {
    [OW_END_RETURN] = RF_STRING_STATIC_INIT("return"),
    [OW_END_PASSED] = RF_STRING_STATIC_INIT("passed")
};

const struct RFstring *ow_node_end_type_str(enum ow_end_type type)
{
    return &ow_endnode_strings[type];
}

const struct RFstring *ow_node_id(const struct ow_node *n)
{
    if (n->type == OW_NTYPE_END) {
        if (n->end.type == OW_END_PASSED) {
            return RFS(
                RFS_PF" to "RFS_PF,
                RFS_PA(ow_node_end_type_str(n->end.type)),
                RFS_PA(n->end.other_fn_name)
            );
        } else {
            return RFS(RFS_PF, RFS_PA(ow_node_end_type_str(n->end.type)));
        }
    }
    return RFS(RFS_PF"_"RFS_PF, RFS_PA(&n->full.val->id), RFS_PA(n->fnname));
}



i_INLINE_INS const void *ownode_objset_key(const struct ow_node *n);

size_t ownode_objset_hashfn(const struct ow_node *n)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    RFS_PUSH();
    size_t ret = rf_hash_str_stable(ow_node_id(n), 0);
    RFS_POP();
    return ret;
}

bool ownode_objset_eqfn(const struct ow_node *n1,
                        const struct ow_node *n2)
{
    RF_ASSERT(n1->type != OW_NTYPE_END, "No end node should appear here");
    RF_ASSERT(n2->type != OW_NTYPE_END, "No end node should appear here");
    RFS_PUSH();
    bool ret = rf_string_equal(ow_node_id(n1), ow_node_id(n2));
    RFS_POP();
    return ret;
}

static bool id_cmp_fn(struct ow_node *n, size_t *id)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    RFS_PUSH();
    size_t nid = rf_hash_str_stable(ow_node_id(n), 0);
    RFS_POP();
    return nid == *id;
}

struct ow_node *ownode_objset_has_value(const struct rf_objset_ownode *set, const struct RFstring *fnname, const struct rir_value *v)
{
    RFS_PUSH();
    const struct RFstring *s = RFS(RFS_PF"_"RFS_PF, RFS_PA(&v->id), RFS_PA(fnname));
    size_t id = rf_hash_str_stable(s, 0);
    OWDD("has_value: Checking for \""RFS_PF"\"\n", RFS_PA(s));
    RFS_POP();
    struct ow_node *ret = htable_get(
        &set->raw.ht,
        id,
        (bool (*)(const void *, void *))id_cmp_fn,
        &id
    );
    if (ret) {
        OWDD("found node in the set: \""RFS_PF"\"\n", RFS_PA(ow_node_id(ret)));
    }
    return ret;
}
