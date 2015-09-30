#include <ir/rir_utils.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>

struct rir_value g_rir_const_1;
struct rir_value g_rir_const_m1;
static bool utils_created = false;

bool rir_utils_create()
{
    if (!rir_constantval_init_fromint(&g_rir_const_1, 1)) {
        return false;
    }
    if (!rir_constantval_init_fromint(&g_rir_const_m1, -1)) {
        return false;
    }
    utils_created = true;
    return true;
}

void rir_utils_destroy()
{
    if (utils_created) {
        rir_value_deinit(&g_rir_const_m1);
        rir_value_deinit(&g_rir_const_1);
    }
    utils_created = false;
}
