#include <utils/data.h>

#include <rflib/defs/threadspecific.h>
#include <rflib/system/system.h>
#include <rflib/string/common.h>
#include <rflib/string/core.h>

static i_THREAD__ struct RFstring *s_data_dir = NULL;

const struct RFstring *rf_data_dir()
{
    if (!s_data_dir) {
        RFS_PUSH();
        const struct RFstring *home = rf_homedir();
        RF_ASSERT_OR_EXIT(home, "Failed to get user's home directory");
        s_data_dir = rf_string_createv(RFS_PF"/.refu", RFS_PA(home));
        RF_ASSERT_OR_EXIT(s_data_dir, "Failed to allocate data dir string");
        RFS_POP();
    }
    return s_data_dir;
}

bool rf_data_dir_exists()
{
    const struct RFstring *datadir = rf_data_dir();
    if (!rf_system_file_exists(datadir)) {
        return rf_system_make_dir(datadir, RFP_IRWXU);
    }
    return true;
}
