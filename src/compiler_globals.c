#include <compiler_globals.h>

#include <String/rf_str_core.h>

static const struct RFstring g_main_str = RF_STRING_STATIC_INIT("main");
const struct RFstring *compiler_main_str()
{
    return &g_main_str;
}
