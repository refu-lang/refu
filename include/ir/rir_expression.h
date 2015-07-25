#ifndef LFR_IR_RIR_EXPRESSION_H
#define LFR_IR_RIR_EXPRESSION_H

#include <RFintrusive_list.h>

struct rir_expression {
    // Control to be added to rir expression list of a block
    struct RFilist_node ln;
};

struct rir_expression *rir_expression_create();
void rir_expression_destroy(struct rir_expression *expr);

#endif
