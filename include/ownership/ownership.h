#ifndef LFR_ONWERSHIP_H
#define LFR_ONWERSHIP_H

#include <stdbool.h>

struct compiler;
struct RFstring;

bool ownership_pass(struct compiler *c);
const struct RFstring *ow_curr_fnname();
void ow_reset_expr_idx();
unsigned int ow_expr_idx_inc();
unsigned int ow_expr_idx();
#endif
