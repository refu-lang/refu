#include "testsupport_parser.h"

#include <info/info.h>
#include <info/msg.h>


#define ck_parserr_check_abort(file_, line_, msg_, ...)                 \
    ck_abort_msg("Checking expected parser error from: %s:%u\n\t"msg_,  \
                 file_, line_, __VA_ARGS__)

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

        // check for error message string
        if (!rf_string_equal(&msg->s, &exp_errors[i].s)) {
            ck_parserr_check_abort(
                filename, line,
                "For parser error number %u: Got:\n\""RF_STR_PF_FMT"\"\n"
                "but expected:\n\""RF_STR_PF_FMT"\"", i,
                RF_STR_PF_ARG(&msg->s), RF_STR_PF_ARG(&exp_errors[i].s));
            return false;
        }

        // check for error message location
        if (!inplocation_mark_equal(&msg->start_mark, &exp_errors[i].start_mark)) {
            ck_parserr_check_abort(
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
            ck_parserr_check_abort(
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
    return true;
}
