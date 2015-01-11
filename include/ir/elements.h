#ifndef LFR_IR_ELEMENTS_H
#define LFR_IR_ELEMENTS_H

// TODO: Total work in progress for elements of the intermediate representation
//       format. For now simply played with the new STRUCT macros here.

#include <Utils/struct_utils.h>

struct rir_basic_block {
    int a;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_basic_block, int a);


#endif
