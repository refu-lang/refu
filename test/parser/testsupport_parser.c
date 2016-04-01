#include "testsupport_parser.h"
#include "../testsupport_front.h"

#include <info/info.h>
#include <info/msg.h>

#include <ast/identifier.h>
#include <ast/ast.h>
#include <ast/ast_utils.h>

#define ck_parser_check_abort(file_, line_, msg_, ...)                 \
    ck_abort_msg("Checking expected parser error from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)


static bool do_finalize_parsing(struct ast_node *n, void* arg)
{
    (void)arg;
    n->state = AST_NODE_STATE_AFTER_PARSING;
    return true;
}
void i_test_finalize_parsing(struct ast_node *n)
{
    ast_pre_traverse_tree(n, do_finalize_parsing, NULL);
}

bool ck_assert_parser_errors_impl(struct info_ctx *info,
                                  struct info_msg *exp_errors,
                                  unsigned num,
                                  const char *filename,
                                  unsigned int line)
{
    struct info_msg *msg;
    struct info_ctx_msg_iterator iter;
    unsigned i = 0;
    info_ctx_get_iter(info, MESSAGE_ANY, &iter);

    while ((msg = info_ctx_msg_iterator_next(&iter))) {
        if (i == num) {
            ck_parser_check_abort(
                filename, line,
                "For parser error number %u: Got:\n\""RFS_PF"\"\n"
                "but expected no error.", i,
                RFS_PA(&msg->s)
            );
            return false;
        }
        // check for error message string
        if (!rf_string_equal(&msg->s, &exp_errors[i].s)) {
            ck_parser_check_abort(
                filename, line,
                "For parser error number %u: Got:\n\""RFS_PF"\"\n"
                "but expected:\n\""RFS_PF"\"", i,
                RFS_PA(&msg->s),
                RFS_PA(&exp_errors[i].s)
            );
            return false;
        }

        // check for error message location
        if (!inplocation_mark_equal(&msg->start_mark, &exp_errors[i].start_mark)) {
            ck_parser_check_abort(
                filename, line,
                "For parser error number %u got different start location marks. Got:\n"
                INPLOCMARKS_FMT " but"
                " expected:\n"INPLOCMARKS_FMT,
                i,
                INPLOCMARKS_ARG(info->file, &msg->start_mark, &msg->end_mark),
                INPLOCMARKS_ARG(info->file, &exp_errors[i].start_mark,
                                &exp_errors[i].end_mark));
            return false;
        }

        if (info_msg_has_end_mark(msg) &&
            !inplocation_mark_equal(&msg->end_mark, &exp_errors[i].end_mark)) {
            ck_parser_check_abort(
                filename, line,
                "For parser error number %u got different end location marks. Got:\n"
                INPLOCMARKS_FMT " but"
                " expected:\n"INPLOCMARKS_FMT,
                i,
                INPLOCMARKS_ARG(info->file, &msg->start_mark, &msg->end_mark),
                INPLOCMARKS_ARG(info->file, &exp_errors[i].start_mark,
                                &exp_errors[i].end_mark));
            return false;
        }


        i ++;
    }


    if (i < num) {
        ck_parser_check_abort(
            filename, line,
            "Got %u parser errors but expected %u.",
            i,
            num
        );
    }

    return true;
}

struct ast_node *testsupport_parser_identifier_create(unsigned int sline,
                                                      unsigned int scol,
                                                      unsigned int eline,
                                                      unsigned int ecol)
{
    struct front_testdriver *d = get_front_testdriver();
    struct inplocation temp_location_ = LOC_INIT(d->current_front->file,
                                                 sline,
                                                 scol,
                                                 eline,
                                                 ecol);
    struct ast_node *n = ast_identifier_create(&temp_location_, 0);
    // since this is testing make sure it's owned by the parser for proper freeing
    n->state = AST_NODE_STATE_AFTER_PARSING;
    return n;
}
