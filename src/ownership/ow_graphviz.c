#include "ow_graph.h"
#include "ow_edge.h"
#include <graphviz/gvc.h>
#include <String/rf_str_conversion.h>

#include <ir/rir_value.h>
#include <ir/rir_object.h>

struct gvz_ctx {
    GVC_t *gvc;
    FILE *f;
    unsigned int gcount;
};
i_THREAD__ struct gvz_ctx *g_gvz_ctx = NULL;

bool ow_graphviz_init()
{
    RF_ASSERT_OR_EXIT(!g_gvz_ctx, "Global graphviz context was already initialized.");
    g_gvz_ctx = malloc(sizeof(struct gvz_ctx));
    RF_ASSERT_OR_EXIT(g_gvz_ctx, "Could not allocate a global graphviz context");
    g_gvz_ctx->gvc = gvContext();
    g_gvz_ctx->gcount = 0;
    return g_gvz_ctx->gvc != NULL;
}

void ow_graphviz_deinit()
{
    RF_ASSERT(g_gvz_ctx, "No global graphviz context exists");
    // Unfortunately even if we do so, there is still a memory leak inside
    // the graphviz library as reported by this bug:
    // http://www.graphviz.org/mantisbt/view.php?id=2438
    gvFreeContext(g_gvz_ctx->gvc);
    free(g_gvz_ctx);
    g_gvz_ctx = NULL;
}

static inline Agnode_t *rf_agnode(Agraph_t *g, const struct RFstring *s)
{
    Agnode_t *ret;
    RFS_PUSH();
    ret = agnode(g, rf_string_cstr_from_buff_or_die(s), 1);
    RFS_POP();
    return ret;
}

static inline Agedge_t *rf_agedge(Agraph_t *g, Agnode_t *from, Agnode_t *to, const struct RFstring *desc)
{
    Agedge_t *ret;
    RFS_PUSH();
    ret = agedge(g, from, to, rf_string_cstr_from_buff_or_die(desc), 1);
    agsafeset(ret, "label", rf_string_cstr_from_buff_or_die(desc), "");
    RFS_POP();
    return ret;
}

static Agnode_t *rf_gv_add(struct ow_node *n, Agraph_t *g)
{
    Agnode_t *agchild;
    Agnode_t *agnode;

    if (!(agnode = rf_agnode(g, ow_node_id(n)))) {
        return NULL;
    }
    struct ow_edge **edge;
    darray_foreach(edge, n->edges) {
        if (!(agchild = rf_gv_add((*edge)->to, g))) {
            return NULL;
        }
        RFS_PUSH();
        rf_agedge(g, agnode, agchild, RFS("%u - "RFS_PF, (*edge)->counter, RFS_PA(rir_expression_type_string((*edge)->edgeexpr))));
        RFS_POP();
    }
    return agnode;
}

bool ow_graph_to_graphviz(struct ow_graph *inputg)
{
    Agraph_t *g;

    // for now open the file here
    FILE *f;
    RFS_PUSH();
    char buff[50];
    char *fnname = rf_string_cstr_from_buff_or_die(inputg->fn_name);
    sprintf(buff, "TESTGRAPH_%s_%u.svg", fnname, g_gvz_ctx->gcount++);
    f = fopen(buff, "w");

    g = agopen(fnname, Agdirected, NULL);
    gvLayout(g_gvz_ctx->gvc, g, "dot");
    rf_gv_add(inputg->root, g);
    gvLayoutJobs(g_gvz_ctx->gvc, g);

    // for now render to the file opened here
    gvRender(g_gvz_ctx->gvc, g, "svg", f);

    gvFreeLayout(g_gvz_ctx->gvc, g);
    agclose(g);

    // for now close the file here
    fclose(f);

    RFS_POP();
    return true;
}
