#include "testsupport_parser.h"

#include <parser/file.h>


struct parser_file testsupport_parser_file;

static struct parser_file *parser_file_dummy_new()
{
    struct parser_file *ret;
    struct RFstringx file_str;
    struct RFarray lines_arr;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    
    ret->info = info_ctx_create();
    if (!ret->info) {
        return NULL;
    }

    if (!rf_string_init(&ret->file_name, "test_file")) {
        return NULL;
    }

    if (!rf_stringx_init_buff(&file_str, 1024, "")) {
        return NULL;
    }

    RF_STRINGX_SHALLOW_COPY(&ret->pstr.str, &file_str);
    ret->pstr.lines_num = 0;
    ret->pstr.lines = NULL;

    ret->current_line = 0;
    ret->current_col = 0;
    return ret;
}

static void parser_file_dummy_free(struct parser_file *f)
{
    parser_file_deinit(f);
    free(f);
}


bool parser_file_dummy_assign(struct parser_file *f, const struct RFstring *s)
{
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    int lines;
    if (!rf_stringx_assign(&f->pstr.str, s)) {
        return false;
    }

    lines = rf_string_count(&f->pstr.str, &nl, 0, 0);
    //TODO: here figure out line byte positions
    return true;
}

void setup_parser_tests()
{
    testsupport_parser_file = *parser_file_dummy_new();
}

void teardown_parser_tests()
{
    parser_file_dummy_free(&testsupport_parser_file);
}
