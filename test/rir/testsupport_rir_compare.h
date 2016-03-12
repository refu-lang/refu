#ifndef LFR_TESTSUPPORT_RIR_COMPARE_H
#define LFR_TESTSUPPORT_RIR_COMPARE_H

struct rir_type;
struct rir_expression;
struct RFstring;

#include <stdbool.h>

bool ckr_compare_type(
    const struct rir_type *got,
    const struct rir_type *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
);

bool ckr_compare_expression(
    struct rir_expression *got,
    struct rir_expression *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
);


#endif
