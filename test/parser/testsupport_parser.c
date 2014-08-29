#include "testsupport_parser.h"

#include <refu.h>
#include <ast/ast.h>
#include <Definitions/threadspecific.h>

i_THREAD__ struct parser_testdriver __parser_testdriver;

struct parser_testdriver *get_parser_testdriver()
{
    return &__parser_testdriver;
}

struct parser_file *parser_testdriver_get_file(struct parser_testdriver *d)
{
    return &d->f;
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
    parser_offset_init(&f->offset);
    return true;
}

static void parser_file_dummy_deinit(struct parser_file *f)
{
    /*
     * if in the test the parser_file_dummy_assign() function
     * was not used then pstr.lines was never allocated so, to still
     * test parser_file_deinit() let's quickly allocated it here
     */
    if (!f->pstr.lines) {
        RF_MALLOC(f->pstr.lines, 1,;);
    }
    parser_file_deinit(f);
}

static bool parser_file_dummy_assign(struct parser_file *f,
                                     const struct RFstring *s)
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
    RF_MALLOC(f->pstr.lines, sizeof(uint32_t) * lines, goto end);
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

bool parser_testdriver_init(struct parser_testdriver *d)
{
    return parser_file_dummy_init(&d->f);
}
void parser_testdriver_deinit(struct parser_testdriver *d)
{
    parser_file_dummy_deinit(&d->f);
}

struct parser_file *parser_testdriver_assign(struct parser_testdriver *d,
                                             const struct RFstring *s)
{
    return parser_file_dummy_assign(&d->f, s) ? &d->f : NULL;
}


void setup_parser_tests()
{
    ck_assert_msg(rf_init("refuclib.log", 0, LOG_DEBUG),
                  "Failed to initialize refu library");
    ck_assert_msg(parser_testdriver_init(&__parser_testdriver),
                  "Failed to initialize parser test driver");
}

void teardown_parser_tests()
{
    parser_testdriver_deinit(&__parser_testdriver);
    rf_deinit();
}


static bool check_nodes(struct ast_node *a, struct ast_node *b)
{
    struct ast_node *child;
    int a_children = 0;
    int b_children = 0;
    if (a->type != b->type) {
        return false;
    }


    rf_ilist_for_each(&a->children, child, lh) {
        a_children ++;
    }
    rf_ilist_for_each(&b->children, child, lh) {
        b_children ++;
    }
    if (a_children != b_children) {
        return false;
    }

    return true;
}

bool check_ast_match(struct ast_node *got,
                     struct ast_node *expect)
{
    struct ast_node *got_child;
    struct ast_node *expect_child;
    int i = 0;
    int j = 0;

    if (!check_nodes(got, expect)) {
        return false;
    }

    rf_ilist_for_each(&got->children, got_child, lh) {

        j = 0;
        rf_ilist_for_each(&expect->children, expect_child, lh) {
            if (i == j && !check_ast_match(got_child, expect_child)) {
                return false;
            }
            j++;
        }
        i ++;
    }
    return true;
}
