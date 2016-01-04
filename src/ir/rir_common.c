#include <ir/rir_common.h>

#include <ir/rir_block.h>
#include <RFintrusive_list.h>

void rir_common_block_add(struct rir_common *c, struct rir_expression *expr)
{
    rf_ilist_add_tail(&c->current_block->expressions, &expr->ln);
}
