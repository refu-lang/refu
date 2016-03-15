#include <info/msg.h>

#include <rflib/utils/memory.h>
#include <rflib/utils/sanity.h>
#include <rflib/utils/build_assert.h>

#include <inpfile.h>

#define INFO_WARNING_STR "warning"
#define INFO_ERROR_STR "error"

#define LOCMARK_FMT "%*c\n"
#define LOCMARK_ARG(column_position_) column_position_ + 1, '^'

#define LOCMARK2_FMT "%*c%*c\n"
#define LOCMARK2_ARG(colpos1_, colpos2_) colpos1_ + 1, '^', \
        colpos2_ - colpos1_, '^'


i_INLINE_INS bool info_msg_has_end_mark(struct info_msg *msg);

struct info_msg *info_msg_create(enum info_msg_type type,
                                 const struct inplocation_mark *start,
                                 const struct inplocation_mark *end,
                                 const char *fmt,
                                 va_list args)
{
    struct info_msg *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);

    if (!rf_string_initvl(&ret->s, fmt, args)) {
        va_end(args);
        return NULL;
    }

    ret->type = type;

    if (start) {
        ret->start_mark = *start;
    } else {
        LOCMARK_RESET(&ret->start_mark);
    }

    if (end) {
        ret->end_mark = *end;
    } else {
        LOCMARK_RESET(&ret->end_mark);
    }

    return ret;
}

void info_msg_destroy(struct info_msg *m)
{
    rf_string_deinit(&m->s);
    free(m);
}

void info_msg_print(struct info_msg *m, FILE *f, struct inpfile *input_file)
{
// TODO: combine with get_formatted
#if 0
    struct RFstring line_str;
    switch(m->type) {
    case MESSAGE_SEMANTIC_WARNING:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RFS_PF"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RFS_PA(&m->s));

        break;
    case MESSAGE_SYNTAX_WARNING:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RFS_PF"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RFS_PA(&m->s));

        break;
    case MESSAGE_SEMANTIC_ERROR:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RFS_PF"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RFS_PA(&m->s));
        break;
    case MESSAGE_SYNTAX_ERROR:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RFS_PF"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RFS_PA(&m->s));
        if (!inpfile_line(input_file, m->loc.start.line, &line_str)) {
            ERROR("Could not locate line %u at file "RFS_PF,
                  m->loc.start.line,
                  RFS_PA(inpfile_name(input_file)));
        } else {
            fprintf(f, RFS_PF, RFS_PA(&line_str));
            print_location_marker(f, m->loc.start.col);
        }
        break;
    default: /* should never get here */
        assert(0);
        break;
    }
#endif
}

bool info_msg_get_formatted(struct info_msg *m, struct RFstringx *s,
                            struct inpfile *input_file)
{
    struct RFstring line_str;
    switch(m->type) {
    case MESSAGE_SEMANTIC_WARNING:
        rf_stringx_assignv(
            s,
            INPLOCMARKS_FMT" "INFO_WARNING_STR": "RFS_PF"\n",
            INPLOCMARKS_ARG(input_file, &m->start_mark, &m->end_mark),
            RFS_PA(&m->s));

        break;
    case MESSAGE_SYNTAX_WARNING:
        rf_stringx_assignv(
            s,
            INPLOCMARKS_FMT" "INFO_WARNING_STR": "RFS_PF"\n",
            INPLOCMARKS_ARG(input_file, &m->start_mark, &m->end_mark),
            RFS_PA(&m->s));

        break;
    case MESSAGE_SEMANTIC_ERROR:
        rf_stringx_assignv(
            s,
            INPLOCMARKS_FMT" "INFO_ERROR_STR": "RFS_PF"\n",
            INPLOCMARKS_ARG(input_file, &m->start_mark, &m->end_mark),
            RFS_PA(&m->s));
        break;
    case MESSAGE_SYNTAX_ERROR:
        rf_stringx_assignv(
            s,
            INPLOCMARKS_FMT" "INFO_ERROR_STR": "RFS_PF"\n",
            INPLOCMARKS_ARG(input_file, &m->start_mark, &m->end_mark),
            RFS_PA(&m->s));
        if (!inpfile_line(input_file, m->start_mark.line, &line_str)) {
            ERROR(
                "Could not locate line %u at file "RFS_PF,
                m->start_mark.line,
                RFS_PA(inpfile_name(input_file))
            );
            return false;
        } else {
            rf_stringx_move_end(s);
            rf_stringx_assignv(s, RFS_PF"\n", RFS_PA(&line_str));
            rf_stringx_move_end(s);

            // set the markers
            if (info_msg_has_end_mark(m)) {
                rf_stringx_assignv(s,
                                   LOCMARK2_FMT,
                                   LOCMARK2_ARG(m->start_mark.col,
                                                m->end_mark.col));
            } else {
                rf_stringx_assignv(s,
                                   LOCMARK_FMT,
                                   LOCMARK_ARG(m->start_mark.col));
            }
        }
        break;
    default: /* should never get here */
        RF_ASSERT_OR_CRITICAL(false, return false,
                              "Illegal compiler message type encountered");
    }

    return true;
}

// keep in sync with enum info_msg_type in info.h
static const struct RFstring info_msg_type_strings[] = {
    RF_STRING_STATIC_INIT("any"),
    RF_STRING_STATIC_INIT("semantic warning"),
    RF_STRING_STATIC_INIT("syntax warning"),
    RF_STRING_STATIC_INIT("semantic error"),
    RF_STRING_STATIC_INIT("syntax error")
};

const struct RFstring *info_msg_type_to_str(enum info_msg_type type)
{
    int i = 0;
    switch (type) {
    case MESSAGE_ANY:
        i = 0;
        break;
    case MESSAGE_SEMANTIC_WARNING:
        i = 1;
        break;
    case MESSAGE_SYNTAX_WARNING:
        i = 2;
        break;
    case MESSAGE_SEMANTIC_ERROR:
        i = 3;
        break;
    case MESSAGE_SYNTAX_ERROR:
        i = 4;
        break;
    default:
        RF_CRITICAL_FAIL("Illegal info_msg_type");
        return NULL;
    }
    return &info_msg_type_strings[i];
}
