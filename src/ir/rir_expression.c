#include <ir/rir_expression.h>
#include <Utils/sanity.h>
#include <ast/ast.h>


static inline void rir_expression_init(struct rir_expression *expr, enum rir_expression_type type)
{
    expr->type = type;
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


static inline void rir_binaryop_init(struct rir_binaryop *op,
                                     const struct rir_expression *a,
                                     const struct rir_expression *b)
{
    op->a = a;
    op->b = b;
}

struct rir_expression *rir_binaryop_create(enum rir_expression_type type,
                                           const struct rir_expression *a,
                                           const struct rir_expression *b)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    rir_expression_init(ret, type);
    rir_binaryop_init(&ret->binaryop, a, b);
    return ret;
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
    rir_expression_init(ret, RIR_EXPRESSION_ALLOCA);
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

