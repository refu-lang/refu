#include "ow_node.h"
#include "ow_edge.h"
#include "ow_debug.h"
#include <ir/rir_object.h>
#include <String/rf_str_core.h>


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

struct ow_node *ow_node_create(const struct RFstring *fnname, const struct rir_value *nodeval)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->fnname = fnname;
    ow_node_init(ret, nodeval);
    return ret;
}

struct ow_node *ow_node_end_create(const struct RFstring *fnname, enum ow_end_type end_type)
{
    struct ow_node *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    ret->fnname = fnname;
    ow_node_init_end(ret, end_type);
    return ret;
}

bool ow_node_connect_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other)
{
    struct ow_edge *e = ow_edge_create_from_node(expr, other);
    if (!e) {
        return false;
    }
    darray_append(n->full.edges, e);
    return true;
}

bool ow_node_connect_end_node(struct ow_node *n, const struct rir_expression *expr, struct ow_node *other)
{
    OWDD("Connect node \""RF_STR_PF_FMT"\" with \""RF_STR_PF_FMT"\"\n",
         RF_STR_PF_ARG(ow_node_id(n)),
         RF_STR_PF_ARG(ow_node_id(other))
    );
    return ow_node_connect_node(n, expr, other);
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
    struct ow_edge *e = ow_edge_create(expr, n->fnname, otherval);
    if (!e) {
        return NULL;
    }
    darray_append(n->full.edges, e);
    return e->to;
}

struct ow_node *ow_node_add_end_edge(struct ow_node *n, enum ow_end_type end_type, const struct rir_expression *expr)
{
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    struct ow_edge *e = ow_endedge_create(expr, n->fnname, end_type);
    if (!e) {
        return NULL;
    }
    darray_append(n->full.edges, e);
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
    RF_ASSERT(n->type != OW_NTYPE_END, "No end node should appear here");
    return RFS(RF_STR_PF_FMT"_"RF_STR_PF_FMT, RF_STR_PF_ARG(&n->full.val->id), RF_STR_PF_ARG(n->fnname));
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
    const struct RFstring *s = RFS(RF_STR_PF_FMT"_"RF_STR_PF_FMT, RF_STR_PF_ARG(&v->id), RF_STR_PF_ARG(fnname));
    size_t id = rf_hash_str_stable(s, 0);
    OWDD("has_value: Checking for \""RF_STR_PF_FMT"\"\n", RF_STR_PF_ARG(s));
    RFS_POP();
    struct ow_node *ret = htable_get(&set->raw.ht,
                                     id,
                                     (bool (*)(const void *, void *))id_cmp_fn,
                                     &id);
    if (ret) {
        OWDD("found node in the set: \""RF_STR_PF_FMT"\"\n", RF_STR_PF_ARG(ow_node_id(ret)));
    }
    return ret;
}
