#include "ow_graph.h"

#include <rfbase/string/core.h>

#include "ow_debug.h"
#include <ownership/ownership.h>
#include <ir/rir_object.h>

struct ow_passed_loc *ow_passed_loc_create(const struct rir_call *c, struct ow_node *n, unsigned int idx)
{
    struct ow_passed_loc *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    RF_STRUCT_ZERO(ret);
    ret->call = c;
    ret->from_node = n;
    ret->node = n;
    ret->idx = idx;
    return ret;
}

void ow_passed_loc_destroy(struct ow_passed_loc *ploc)
{
    free(ploc);
}

struct ow_graph *ow_graph_create(struct rir_object *obj,
                                 const struct RFstring *name,
                                 struct symbol_table_record *rec)
{
    struct ow_graph *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    RF_STRUCT_ZERO(ret);
    ret->fn_name = name;
    ret->obj = obj;
    ret->root = ow_node_create(name, rir_object_value(obj));
    ret->graph_attrs = 0;
    ret->rec = rec;
    if (!ret->root) {
        return false;
    }
    darray_init(ret->passed_locations);
    rf_objset_init(&ret->set, ownode);
    OWDD("\n>>>Creating graph for value "RFS_PF"\n\n", RFS_PA(ow_node_id(ret->root)));
    rf_objset_add(&ret->set, ownode, ret->root);
    return ret;
}

void ow_graph_destroy(struct ow_graph *g)
{
    struct ow_passed_loc **ploc;
    darray_foreach(ploc, g->passed_locations) {
        ow_passed_loc_destroy(*ploc);
    }
    darray_free(g->passed_locations);
    ow_node_destroy(g->root);
    rf_objset_clear(&g->set);
    free(g);
}

bool ow_graph_check_or_add_val(struct ow_graph *g,
                               const struct rir_value *v,
                               const struct rir_value *dependentv,
                               const struct rir_expression *edgexpr)
{
    struct ow_node *n;
    OWDD(
        "Normal Checking in \""RFS_PF"()\" if value "RFS_PF" can go to graph for value "RFS_PF"\n",
        RFS_PA(ow_curr_fnname()),
        RFS_PA(rir_value_string(v)),
        RFS_PA(ow_node_id(g->root))
    );
    if (!(n = ownode_objset_has_value(&g->set, ow_curr_fnname(), v))) {
        OWDD("No it can't!\n");
        return false;
    }
    OWDD("Yes it can.\n");
    if (!(n = ow_node_add_val_edge(n, dependentv, edgexpr))) {
        OWDD("Failed to add a new node as an edge");
        return false;
    }
    OWDD("Adding node with id \""RFS_PF"\" to the graph for value "RFS_PF"\n",
       RFS_PA(ow_node_id(n)),
       RFS_PA(ow_node_id(g->root))
    );
    rf_objset_add(&g->set, ownode, n);
    return true;
}

bool ow_graph_check_or_add_end(struct ow_graph *g,
                               const struct rir_value *v,
                               enum ow_end_type end_type,
                               const struct rir_expression *edgexpr,
                               unsigned int idx)
{
    struct ow_node *n;
    OWDD(
        "End Checking in \""RFS_PF"()\" if value "RFS_PF" can go to graph for value "RFS_PF"\n",
        RFS_PA(ow_curr_fnname()),
        RFS_PA(rir_value_string(v)),
        RFS_PA(ow_node_id(g->root))
    );
    if (!(n = ownode_objset_has_value(&g->set, ow_curr_fnname(), v))) {
        OWDD("No it can't!\n");
        return false;
    }
    OWDD("Yes it can.\n");
    if (!(n = ow_node_add_end_edge(n, end_type, edgexpr))) {
        OWDD("Failed to add a new node as an edge");
        return false;
    }
    switch (end_type) {
    case OW_END_RETURN:
        OWDD("End Checking was for RETURN\n");
        ow_graph_set_attr(g, OW_ATTR_RETURNED);
        break;
    case OW_END_PASSED:
    {
        OWDD("End Checking was for CALL\n");
        ow_graph_set_attr(g, OW_ATTR_PASSED);
        RF_ASSERT(edgexpr->type == RIR_EXPRESSION_CALL, "A rir call should be here");
        struct ow_passed_loc *ploc = ow_passed_loc_create(&edgexpr->call, n, idx);
        darray_append(g->passed_locations, ploc);
    }
    break;
    }
    return true;
}

void ow_graph_set_attr(struct ow_graph *g, enum graph_attrs attr)
{
    switch(attr)  {
    case OW_ATTR_RETURNED:
        if (g->obj->category == RIR_OBJ_EXPRESSION && g->obj->expr.type == RIR_EXPRESSION_ALLOCA) {
            // since the graph states the value is returned, allocation needs to be in the heap
            g->obj->expr.alloca.alloc_location = RIR_ALLOC_HEAP;
        }
    break;
    case OW_ATTR_PASSED:
        break;
    }
    RF_BITFLAG_SET(g->graph_attrs, attr);
}

i_INLINE_INS bool ow_graph_has_attr(const struct ow_graph *g, enum graph_attrs attr);
