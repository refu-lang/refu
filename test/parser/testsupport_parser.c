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
        if(!inplocation_equal(&msg->loc, &exp_errors[i].loc)) {
            ck_parserr_check_abort(
                filename, line,
                "For parser error number %u got different locations. Got:\n"
                INPLOCATION_FMT2 " but"
                " expected:\n"INPLOCATION_FMT2,
                i,
                INPLOCATION_ARG2(info->file, &msg->loc),
                INPLOCATION_ARG2(info->file, &exp_errors[i].loc));
        
        return false;
        }
        
        i ++;
    }
    return true;
}
