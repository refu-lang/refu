#include <ir/rir_expression.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


static bool rir_expression_init(struct rir_expression *expr, struct ast_node *n, struct rir_ctx *ctx)
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

struct rir_expression *rir_expression_create(struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_expression_init(ret, n, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_expression_deinit(struct rir_expression *expr)
{
    // TODO
}

void rir_expression_destroy(struct rir_expression *expr)
{
    rir_expression_deinit(expr);
    free(expr);
}



static inline bool rir_alloca_init(struct rir_alloca *obj,
                                  const struct rir_type *type,
                                  uint64_t num)
{
    obj->type = type;
    obj->num = num;
    return true;
}

struct rir_expression *rir_alloca_create(const struct rir_type *type, uint64_t num)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_alloca_init(&ret->alloca, type, num)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static inline void rir_alloca_deinit(struct rir_expression *obj)
{
    return;// TODO
}

