#include "testsupport_front.h"

#include <refu.h>
#include <ast/ast.h>
#include <Definitions/threadspecific.h>

i_THREAD__ struct front_testdriver __front_testdriver;

struct front_testdriver *get_front_testdriver()
{
    return &__front_testdriver;
}

struct inpfile *front_testdriver_get_file(struct front_testdriver *d)
{
    return &d->f;
}
static bool inpfile_dummy_init(struct inpfile *f, struct info_ctx *info)
{
    if (!rf_string_init(&f->file_name, "test_file")) {
        return false;
    }

    if (!rf_stringx_init_buff(&f->str.str, 1024, "")) {
        return false;
    }

    f->info = info;
    f->root = NULL;
    inpoffset_init(&f->offset);
    return true;
}

static void inpfile_dummy_deinit(struct inpfile *f)
{
    /*
     * if in the test the inpfile_dummy_assign() function
     * was not used then str.lines was never allocated so, to still
     * use inpfile_deinit() let's quickly allocate it here
     */
    if (!f->str.lines) {
        RF_MALLOC(f->str.lines, 1,;);
    }
    inpfile_deinit(f);
}

static bool inpfile_dummy_assign(struct inpfile *f,
                                 const struct RFstring *s)
{
    bool ret = false;
    struct RFarray arr;
    RF_ARRAY_TEMP_INIT(&arr, uint32_t, 128);
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    int lines;

    if (!rf_stringx_assign(&f->str.str, s)) {
        goto end;
    }
    lines = rf_string_count(&f->str.str, &nl, 0, &arr, 0);
    if (lines == -1) {
        goto end;
    }

    lines += 1;
    RF_MALLOC(f->str.lines, sizeof(uint32_t) * lines, goto end);
    if (lines == 1) { //we got nothing to copy from, so don't
        f->str.lines[0] = 0;
    } else {
        unsigned int i;
        f->str.lines[0] = 0;
        for (i = 1; i < lines; i ++) {
            f->str.lines[i] = rf_array_at_unsafe(&arr, i - 1, uint32_t) + 1;
        }
    }
    f->str.lines_num = lines;

    ret = true;
end:
    rf_array_deinit(&arr);
    return ret;
}

struct RFstringx *front_testdriver_geterrors(struct front_testdriver *d)
{
    if (!info_ctx_get(d->f.info, MESSAGE_ANY, &d->buffstr)) {
        return NULL;
    }
    return &d->buffstr;
}

bool front_testdriver_init(struct front_testdriver *d)
{
    bool ret;
    ret = inpfile_dummy_init(&d->f, d->info);
    if (!ret) {
        return false;
    }
    d->info = info_ctx_create(&d->f);
    if (!d->info) {
        return false;
    }
    darray_init(d->nodes);
    return rf_stringx_init_buff(&d->buffstr, 1024, "");
}
void front_testdriver_deinit(struct front_testdriver *d)
{
    struct ast_node **n;
    rf_stringx_deinit(&d->buffstr);
    darray_foreach(n, d->nodes) {
        ast_node_destroy(*n);
    }
    darray_free(d->nodes);
    info_ctx_destroy(d->info);
    inpfile_dummy_deinit(&d->f);
}

struct inpfile *front_testdriver_assign(struct front_testdriver *d,
                                        const struct RFstring *s)
{
    return inpfile_dummy_assign(&d->f, s) ? &d->f : NULL;
}

struct ast_node *front_testdriver_generate_identifier(
    struct front_testdriver *d,
    struct inplocation *loc,
    const char *s)
{
    struct ast_node *ret;
    // Temporary, till I pass actual location values here
    struct inplocation temp_loc = {{0}, {0}};
    ret = ast_node_create_loc(AST_IDENTIFIER, &temp_loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->identifier.string, (char*)s, strlen(s));
    darray_append(d->nodes, ret);
    return ret;
}


void setup_front_tests()
{
    ck_assert_msg(rf_init("refuclib.log", 0, LOG_DEBUG),
                  "Failed to initialize refu library");
    ck_assert_msg(front_testdriver_init(&__front_testdriver),
                  "Failed to initialize front end test driver");
}

void teardown_front_tests()
{
    front_testdriver_deinit(&__front_testdriver);
    rf_deinit();
}


static bool check_nodes(struct ast_node *got, struct ast_node *expect,
                        struct inpfile *ifile,
                        const char* filename,
                        unsigned int line)
{
    struct ast_node *child;
    int got_children = 0;
    int expect_children = 0;
    if (got->type != expect->type) {
        ck_astcheck_abort(
            filename, line,
            "2 ast nodes have different type: Got \""RF_STR_PF_FMT"\" != "
            "expected \""RF_STR_PF_FMT"\"", RF_STR_PF_ARG(ast_node_str(got)),
            RF_STR_PF_ARG(ast_node_str(expect)));
        return false;
    }

    rf_ilist_for_each(&got->children, child, lh) {
        got_children ++;
    }
    rf_ilist_for_each(&expect->children, child, lh) {
        expect_children ++;
    }
    if (got_children != expect_children) {
        ck_astcheck_abort(filename, line,
                          "2 ast nodes have different number of children", 0);
        return false;
    }

    if(!inplocation_equal(&got->location, &expect->location)) {
        ck_astcheck_abort(
            filename, line,
            "2 ast nodes have different location: Got "INPLOCATION_FMT2 " but"
            " expected "INPLOCATION_FMT2,
            INPLOCATION_ARG2(ifile, &got->location),
            INPLOCATION_ARG2(ifile, &expect->location)
        );
        return false;
    }

    switch(got->type) {
    case AST_IDENTIFIER:
        if (!rf_string_equal(ast_identifier_str(got),
                             ast_identifier_str(expect))) {
            ck_astcheck_abort(
                filename, line,
                "identifiers mismatch: Got \""RF_STR_PF_FMT"\" != expected "
                "\""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(ast_identifier_str(got)),
                RF_STR_PF_ARG(ast_identifier_str(expect))
            );
            return false;
        }
        break;
    default:
        break;
    }

    return true;
}

bool check_ast_match_impl(struct ast_node *got,
                          struct ast_node *expect,
                          struct inpfile *ifile,
                          const char* file,
                          unsigned int line)
{
    struct ast_node *got_child;
    struct ast_node *expect_child;
    int i = 0;
    int j = 0;

    if (!check_nodes(got, expect, ifile, file, line)) {
        return false;
    }

    rf_ilist_for_each(&got->children, got_child, lh) {

        j = 0;
        rf_ilist_for_each(&expect->children, expect_child, lh) {
            if (i == j &&
                !check_ast_match_impl(got_child, expect_child, ifile, file, line)) {
                return false;
            }
            j++;
        }
        i ++;
    }
    return true;
}
