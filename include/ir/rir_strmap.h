#ifndef LFR_RIR_STRMAP_H
#define LFR_RIR_STRMAP_H

#include <Data_Structures/strmap.h>

struct rir_expression;

struct rirexpr_strmap {
    STRMAP_MEMBERS(struct rir_expression *);
};

#endif
