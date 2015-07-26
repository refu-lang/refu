#include <ir/rir_expression.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


static bool rir_expression_init(struct rir_expression *expr, struct ast_node *n, struct rir *r)
{
    switch (n->type) {
    case AST_BINARY_OPERATOR:
        // TODO
        break;
    default:
        RF_ASSERT(false,
                  "Asked to create rir expression from a "RF_STR_PF_FMT" node",
                  RF_STR_PF_ARG(ast_node_str(n)));
        return false;
    }
    return true;
}

struct rir_expression *rir_expression_create(struct ast_node *n, struct rir *r)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, n, r)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_expression_deinit(struct rir_expression *expr)
{

}

void rir_expression_destroy(struct rir_expression *expr)
{
    rir_expression_deinit(expr);
    free(expr);
}
