#ifndef LFR_IR_UTILS_H
#define LFR_IR_UTILS_H

#include <stdbool.h>
#include <ir/rir_value.h>

extern struct rir_value g_rir_const_1;
extern struct rir_value g_rir_const_m1;

/**
 * Initialize all the utils needed by all rir functions
 */
bool rir_utils_create();
void rir_utils_destroy();

#endif
