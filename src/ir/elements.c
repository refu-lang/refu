#include <ir/elements.h>

/* RF_STRUCT_ALLOC_SIG(rir_basic_block) */
/* { */
/*     struct rir_basic_block *ret; */
/*     RF_MALLOC(ret, sizeof(*ret), return NULL); */
/*     return ret; */
/* } */

RF_STRUCT_INIT_SIG(rir_basic_block, int a)
{
    this->a = a;
    return true;
}

RF_STRUCT_DEINIT_SIG(rir_basic_block)
{
    return;
}

/* RF_STRUCT_DEALLOC_SIG(rir_basic_block) */
/* { */
/*     free(this); */
/* } */

/* RF_STRUCT_COMMON_DEFS_WITH_ALLOC(rir_basic_block, int, a) */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_basic_block, int, a)




