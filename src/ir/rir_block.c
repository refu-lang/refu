#include <ir/rir_block.h>
#include <Utils/memory.h>
#include <ast/ast.h>

static bool rir_block_init(struct rir_block *b, const struct ast_node *n)
{
    RF_STRUCT_ZERO(b);
    return true;
}

struct rir_block *rir_block_create(const struct ast_node *n)
{
    struct rir_block *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_block_init(ret, n)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_block_deinit(struct rir_block* b)
{
    if (b->next_block) {
        rir_block_destroy(b->next_block);
    }
}

void rir_block_destroy(struct rir_block* b)
{
    rir_block_deinit(b);
    free(b);
}
