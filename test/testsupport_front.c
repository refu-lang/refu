#include "testsupport_front.h"

#include <refu.h>
#include <Definitions/threadspecific.h>

#include <ast/type.h>
#include <ast/string_literal.h>
#include <ast/constant_num.h>
#include <ast/operators.h>
#include <ast/vardecl.h>
#include <ast/block.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <analyzer/analyzer.h>

#include <stdarg.h>

i_THREAD__ struct front_testdriver __front_testdriver;

struct front_testdriver *get_front_testdriver()
{
    return &__front_testdriver;
}

struct inpfile *front_testdriver_get_file(struct front_testdriver *d)
{
    return d->front.file;
}

static struct inpfile *inpfile_dummy_create(struct info_ctx *info)
{
    struct inpfile *f;
    RF_MALLOC(f, sizeof(*f), return NULL);
    if (!rf_string_init(&f->file_name, "test_file")) {
        return false;
    }

    if (!rf_stringx_init_buff(&f->str.str, 1024, "")) {
        return false;
    }

    f->info = info;
    f->root = NULL;
    inpoffset_init(&f->offset);
    return f;
}

static void inpfile_dummy_destroy(struct inpfile *f)
{
    /*
     * if in the test the inpfile_dummy_assign() function
     * was not used then str.lines was never allocated so, to still
     * use inpfile_deinit() let's quickly allocate it here
     */
    if (!f->str.lines) {
        RF_MALLOC(f->str.lines, 1,;);
    }
    /* inpfile_deinit(f); */
    inpfile_destroy(f);
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
        int i;
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
    if (!info_ctx_get_messages_fmt(d->front.info, MESSAGE_ANY, &d->buffstr)) {
        return NULL;
    }
    return &d->buffstr;
}

bool front_testdriver_init(struct front_testdriver *d)
{
    RF_STRUCT_ZERO(d);
    darray_init(d->nodes);

    // Note: Here are providing NULL pointer for front context input file
    // The input file is created and injected at lexer and parser during
    // front_testdriver_assign()

    if (!rf_stringx_init_buff(&d->buffstr, 1024, "")) {
        goto free_nodes;
    }

    d->front.info = info_ctx_create(d->front.file);
    if (!d->front.info) {
        goto free_buff;
    }

    d->front.lexer = lexer_create(d->front.file, d->front.info);
    if (!d->front.lexer) {
        goto free_info;
    }

    d->front.parser = parser_create(d->front.file,
                                    d->front.lexer,
                                    d->front.info);
    if (!d->front.parser) {
        goto free_lexer;
    }

    d->front.analyzer = analyzer_create(d->front.info);
    if (!d->front.analyzer) {
        goto free_parser;
    }

    return true;

free_parser:
    parser_destroy(d->front.parser);
free_lexer:
    lexer_destroy(d->front.lexer);
free_info:
    info_ctx_destroy(d->front.info);
free_buff:
    rf_stringx_deinit(&d->buffstr);
free_nodes:
    darray_free(d->nodes);
    return false;
}
void front_testdriver_deinit(struct front_testdriver *d)
{
    struct ast_node **n;
    rf_stringx_deinit(&d->buffstr);
    darray_foreach(n, d->nodes) {
        ast_node_destroy(*n);
    }
    darray_free(d->nodes);

    if (d->front.file) {
        inpfile_dummy_destroy(d->front.file);
    }

    lexer_destroy(d->front.lexer);
    parser_destroy(d->front.parser);
    info_ctx_destroy(d->front.info);
    analyzer_destroy(d->front.analyzer);
}

struct front_ctx *front_testdriver_assign(struct front_testdriver *d,
                                          const struct RFstring *s)
{
    d->front.file = inpfile_dummy_create(d->front.info);

    if (!d->front.file) {
        ck_abort_msg("Failed to create a dummy input file");
    }

    if (!inpfile_dummy_assign(d->front.file, s)) {
        ck_abort_msg("Assigning a string to a test driver failed");
    }

    lexer_inject_input_file(d->front.lexer, d->front.file);
    parser_inject_input_file(d->front.parser, d->front.file);

    return &d->front;
}


static inline struct ast_node *front_testdriver_node_from_loc(
    struct front_testdriver *d, enum ast_type type,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec)
{
    struct inplocation temp_loc = LOC_INIT(d->front.file, sl, sc, el, ec);
    return ast_node_create_loc(type, &temp_loc);
}

struct ast_node *front_testdriver_generate_identifier(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    const char *s)
{
    struct ast_node *ret;
    ret = front_testdriver_node_from_loc(d, AST_IDENTIFIER, sl, sc, el, ec);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->identifier.string, (char*)s, strlen(s));
    darray_append(d->nodes, ret);
    return ret;
}

struct ast_node *front_testdriver_generate_string_literal(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    unsigned int sl_byte_off, unsigned int el_byte_off,
    const char *s)
{
    struct ast_node *ret;
    struct inplocation temp_loc = LOC_INIT_FULL(
        sl, sc, el, ec,
        inpfile_line_p(d->front.file, sl) + sl_byte_off,
        inpfile_line_p(d->front.file, el) + el_byte_off);
    ret = ast_string_literal_create(&temp_loc);
    if (!ret) {
        return NULL;
    }
    darray_append(d->nodes, ret);
    return ret;
}

struct ast_node *front_testdriver_generate_constant_float(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    double val)
{
    struct ast_node *ret;
    struct inplocation temp_loc = LOC_INIT(d->front.file, sl, sc, el, ec);
    ret = ast_constantnum_create_float(&temp_loc, val);
    if (!ret) {
        return NULL;
    }
    darray_append(d->nodes, ret);
    return ret;
}

struct ast_node *front_testdriver_generate_constant_integer(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    uint64_t val)
{
    struct ast_node *ret;
    struct inplocation temp_loc = LOC_INIT(d->front.file, sl, sc, el, ec);
    ret = ast_constantnum_create_integer(&temp_loc, val);
    if (!ret) {
        return NULL;
    }
    darray_append(d->nodes, ret);
    return ret;
}

void front_testdriver_node_remove_children_from_array(struct front_testdriver *d,
                                                      struct ast_node *n)
{
    // extremey inefficient but this is just testing code
    struct ast_node *child;
    struct ast_node **arr_n;
    unsigned int i;
    rf_ilist_for_each(&n->children, child, lh) {

        front_testdriver_node_remove_children_from_array(d, child);

        i = 0;
        darray_foreach(arr_n, d->nodes) {
            if (*arr_n == child) {
                darray_remove(d->nodes, i);
                break;
            }
            ++i;
        }
    }
}

struct ast_node *do_front_testdriver_generate_node(
    struct front_testdriver *d,
    unsigned int sl, unsigned int sc, unsigned int el, unsigned int ec,
    enum ast_type type, unsigned int args_num, ...)
{
    va_list args;
    struct ast_node *ret;
    struct ast_node *n1 = NULL;
    struct ast_node *n2 = NULL;
    struct inplocation temp_loc = LOC_INIT(d->front.file, sl, sc, el, ec);
    struct inplocation_mark *smark = &temp_loc.start;
    struct inplocation_mark *emark = &temp_loc.end;
    bool is_constant = false;

    va_start(args, args_num);

    switch(type) {
    case AST_XIDENTIFIER:
        ck_assert_uint_gt(args_num, 0);
        n1 = front_testdriver_generate_identifier(d, sl, sc, el, ec,
                                                 va_arg(args, const char *));
        if (args_num > 1) {
            is_constant = va_arg(args, bool);
        }

        if (args_num > 2) {
            n2 = va_arg(args, struct ast_node *);
        }
        ret = ast_xidentifier_create(smark, emark, n1, is_constant, n2);
        break;
    case AST_TYPE_DESCRIPTION:
        ck_assert_uint_gt(args_num, 1);
        n1 = va_arg(args, struct ast_node *);
        n2 = va_arg(args, struct ast_node *);
        ret = ast_typedesc_create(smark, emark, n1, n2);
        break;
    case AST_VARIABLE_DECLARATION:
        ck_assert_uint_gt(args_num, 0);
        n1 = va_arg(args, struct ast_node *);
        ret = ast_vardecl_create(smark, emark, n1);
        break;
    default:
        ck_assert_msg("invalid type provided to front test driver node generation");
        return NULL;
    }
    va_end(args);

    // extremey inefficient but this is just testing code
    front_testdriver_node_remove_children_from_array(d, ret);

    darray_append(d->nodes, ret);
    return ret;
}


void setup_front_tests()
{
    ck_assert_msg(rf_init(LOG_TARGET_STDOUT, NULL, LOG_DEBUG),
                  "Failed to initialize refu library");
    ck_assert_msg(front_testdriver_init(&__front_testdriver),
                  "Failed to initialize front end test driver");
}

void setup_front_tests_with_file_log()
{
    ck_assert_msg(rf_init(LOG_TARGET_FILE, "refu.log", LOG_DEBUG),
                  "Failed to initialize refu library");
    ck_assert_msg(front_testdriver_init(&__front_testdriver),
                  "Failed to initialize front end test driver");
}

void teardown_front_tests()
{
    front_testdriver_deinit(&__front_testdriver);
    rf_deinit();
}



#define ck_astcheck_abort(file_, line_, msg_, ...)      \
    ck_abort_msg("Checking ast trees from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)
static bool check_block_node(struct ast_node *got, struct ast_node *expect,
                             struct inpfile *ifile,
                             const char* filename,
                             unsigned int line)
{
    // for blocks check the value node has correctly been parsed
    struct ast_node *val_got = ast_block_valueexpr_get(got);
    struct ast_node *val_expect = ast_block_valueexpr_get(expect);

    if (val_got == NULL || val_expect == NULL) {
        if (val_got != val_expect) {
            ck_astcheck_abort(filename, line,
                              "block value expression mismatch for block at"
                              INPLOCATION_FMT2,
                              INPLOCATION_ARG2(ifile, &got->location));
        }
        return true;
    }
    if (!check_ast_match_impl(ast_block_valueexpr_get(got),
                              ast_block_valueexpr_get(expect),
                              ifile, filename, line)) {
        ck_astcheck_abort(filename, line,
                          "block value expression mismatch for block at"
                          INPLOCATION_FMT2,
                          INPLOCATION_ARG2(ifile, &got->location));
    }
    return true;

}
static bool check_nodes(struct ast_node *got, struct ast_node *expect,
                        struct inpfile *ifile,
                        const char* filename,
                        unsigned int line)
{
    enum constant_type ctype;
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
                          "2 \""RF_STR_PF_FMT"\" ast nodes have different "
                          "number of children.\n Got %d != Expected %d",
                          RF_STR_PF_ARG(ast_node_str(got)),
                          got_children, expect_children);
        return false;
    }

    if(!inplocation_equal(&got->location, &expect->location)) {
        ck_astcheck_abort(
            filename, line,
            "2 \""RF_STR_PF_FMT"\" ast nodes have different location: "
            "Got "INPLOCATION_FMT2 " but"
            " expected "INPLOCATION_FMT2, RF_STR_PF_ARG(ast_node_str(got)),
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
    case AST_TYPE_OPERATOR:
        if (ast_typeop_op(got) != ast_typeop_op(expect)) {

            ck_astcheck_abort(
                filename, line,
                "type operator mismatch: Got \""RF_STR_PF_FMT"\" != expected "
                "\""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(ast_typeop_opstr(got)),
                RF_STR_PF_ARG(ast_typeop_opstr(expect)));
        }
        break;
    case AST_BINARY_OPERATOR:
        if (ast_binaryop_op(got) != ast_binaryop_op(expect)) {

            ck_astcheck_abort(
                filename, line,
                "binary operator mismatch: Got \""RF_STR_PF_FMT"\" != expected "
                "\""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(ast_binaryop_opstr(got)),
                RF_STR_PF_ARG(ast_binaryop_opstr(expect)));
        }
        break;
    case AST_STRING_LITERAL:
        if (!rf_string_equal(ast_string_literal_get_str(expect),
                             ast_string_literal_get_str(got))) {

            ck_astcheck_abort(
                filename, line,
                "string literal mismatch: Got \""RF_STR_PF_FMT"\" != expected "
                "\""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(ast_string_literal_get_str(got)),
                RF_STR_PF_ARG(ast_string_literal_get_str(expect)));
        }
        break;
    case AST_BLOCK:
        return check_block_node(got, expect, ifile, filename, line);
    case AST_CONSTANT_NUMBER:
        ctype = ast_constantnum_get_type(got);
        if (ctype != ast_constantnum_get_type(expect)) {
            ck_astcheck_abort(filename, line,
                              "constant number type mismatch for token at "
                              INPLOCATION_FMT2,
                              INPLOCATION_ARG2(ifile, &got->location));
        }

        if (ctype == CONSTANT_NUMBER_INTEGER) {
            uint64_t expect_v;
            uint64_t got_v;
            ck_assert(ast_constantnum_get_integer(expect, &expect_v));
            ck_assert(ast_constantnum_get_integer(got, &got_v));
            if (expect_v != got_v) {
                ck_astcheck_abort(
                    filename, line,
                    "constant integer mismatch: Got \"%"PRIu64"\" != expected \""
                    "%"PRIu64"\"",
                    got_v, expect_v);
            }
        } else if (ctype == CONSTANT_NUMBER_FLOAT) {
            double expect_v;
            double got_v;
            ck_assert(ast_constantnum_get_float(expect, &expect_v));
            ck_assert(ast_constantnum_get_float(got, &got_v));
            if (!DBLCMP_EQ(expect_v, got_v)) {
                ck_astcheck_abort(
                    filename, line,
                    "constant float mismatch: Got \"%f\" != expected \"%f\"",
                    got_v, expect_v);
            }
        } else {
            ck_astcheck_abort(filename, line, "unexpected constant number "
                              "type for token at "INPLOCATION_FMT2,
                              INPLOCATION_ARG2(ifile, &got->location));
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
