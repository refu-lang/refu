#include <ir/rir_utils.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>

struct rir_value g_rir_const_1_actual;
struct rir_value g_rir_const_m1_actual;

bool rir_utils_create()
{
    if (!rir_constantval_init_fromint(&g_rir_const_1_actual, 1)) {
        return false;
    }
    g_rir_const_1 = &g_rir_const_1_actual;
    if (!rir_constantval_init_fromint(&g_rir_const_m1_actual, -1)) {
        return false;
    }
    g_rir_const_m1 = &g_rir_const_m1_actual;
    return true;
}

void rir_utils_destroy()
{
    // anything here? if you add anything, call this to cleanup after create()
}
