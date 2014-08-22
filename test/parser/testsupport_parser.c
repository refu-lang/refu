#include "testsupport_parser.h"

#include <refu.h>
#include <Definitions/threadspecific.h>

#include <parser/file.h>


i_THREAD__ struct parser_file testsupport_parser_file;

struct parser_file *parser_file_dummy_get()
{
    return &testsupport_parser_file;
}
static bool parser_file_dummy_init(struct parser_file *f)
{
    struct RFstringx file_str;
    struct RFarray lines_arr;

    f->info = info_ctx_create();
    if (!f->info) {
        return false;
    }

    if (!rf_string_init(&f->file_name, "test_file")) {
        return false;
    }

    if (!rf_stringx_init_buff(&f->pstr.str, 1024, "")) {
        return false;
    }

    f->root = NULL;
    f->current_line = 0;
    f->current_col = 0;
    return true;
}

static void parser_file_dummy_deinit(struct parser_file *f)
{
    parser_file_deinit(f);
}

bool parser_file_dummy_assign(struct parser_file *f, const struct RFstring *s)
{
    bool ret = false;
    struct RFarray arr;
    RF_ARRAY_TEMP_INIT(&arr, uint32_t, 128);
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    int lines;

    if (!rf_stringx_assign(&f->pstr.str, s)) {
        goto end;
    }
    lines = rf_string_count(&f->pstr.str, &nl, 0, &arr, 0);
    if (lines == -1) {
        goto end;
    }

    lines += 1;
    RF_MALLOC_JMP(f->pstr.lines, sizeof(uint32_t) * lines, ;, end);
    if (lines == 1) { //we got nothing to copy from, so don't
        f->pstr.lines[0] = 0;
    } else {
        memcpy(f->pstr.lines, arr.buff, sizeof(uint32_t) * lines);
    }
    f->pstr.lines_num = lines;

    ret = true;
end:
    rf_array_deinit(&arr);
    return ret;
}

void setup_parser_tests()
{
    rf_init("refuclib.log", 0, LOG_DEBUG);
    parser_file_dummy_init(&testsupport_parser_file);
}

void teardown_parser_tests()
{
    parser_file_dummy_deinit(&testsupport_parser_file);
    rf_deinit();
}
