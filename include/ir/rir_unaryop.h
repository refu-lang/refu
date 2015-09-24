#ifndef LFR_IR_RIR_UNARYOP_H
#define LFR_IR_RIR_UNARYOP_H

#include <stdbool.h>

struct rir_ctx;
struct ast_unaryop;

bool rir_process_unaryop(const struct ast_unaryop *n,
                         struct rir_ctx *ctx);

#endif
