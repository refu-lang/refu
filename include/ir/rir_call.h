#ifndef LFR_IR_FNCALL
#define LFR_IR_FNCALL

#include <stdbool.h>
struct ast_node;
struct rir_ctx;

struct rir_call {
    
};

bool rir_process_fncall(const struct ast_node *n, struct rir_ctx *ctx);

#endif
