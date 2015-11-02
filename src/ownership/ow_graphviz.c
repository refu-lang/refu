#include "ow_graph.h"

#include <graphviz/gvc.h>

bool ow_graph_to_graphviz(struct ow_graph *inputg)
{
    Agraph_t *g;
    Agnode_t *n, *m;
    Agedge_t *e;
    GVC_t *gvc;

    gvc = gvContext();

    /* gvLayout(gvc, g, "dot"); */
    g = agopen("g", Agdirected, NULL);
    gvLayout(gvc, g, "dot");
    n = agnode(g, "n", 1);
    m = agnode(g, "m", 1);
    e = agedge(g, n, m, 0, 1);
    (void)e;
    // Compute a layout using layout engine from command line args
    gvLayoutJobs(gvc, g);

    FILE *f = fopen("TESTGRAPH.svg", "w");
    gvRender(gvc, g, "svg", f);
    fclose(f);


    gvFreeLayout(gvc, g);
    agclose(g);
    gvFreeContext(gvc);
    return true;
}
