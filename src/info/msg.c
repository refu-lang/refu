#include <info/msg.h>

#include <Utils/memory.h>
#include <Utils/sanity.h>
#include <inpfile.h>

#define INFO_WARNING_STR "warning"
#define INFO_ERROR_STR "error"

#define LOCMARK_FMT "%*c\n"
#define LOCMARK_ARG(column_position_) column_position_ + 1, '^'

inline static void print_location_marker(FILE *f, unsigned int col_pos)
{
    printf(LOCMARK_FMT, LOCMARK_ARG(col_pos));
}

struct info_msg *info_msg_create(enum info_msg_type type,
                                 struct inplocation *loc,
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
    inplocation_copy(&ret->loc, loc);

    return ret;
}

void info_msg_destroy(struct info_msg *m)
{
    rf_string_deinit(&m->s);
    free(m);
}

void info_msg_print(struct info_msg *m, FILE *f, struct inpfile *input_file)
{
    struct RFstring line_str;
    switch(m->type) {
    case MESSAGE_SEMANTIC_WARNING:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));

        break;
    case MESSAGE_SYNTAX_WARNING:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));

        break;
    case MESSAGE_SEMANTIC_ERROR:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));
        break;
    case MESSAGE_SYNTAX_ERROR:
        fprintf(
            f,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));
        if (!inpfile_line(input_file, m->loc.start.line, &line_str)) {
            ERROR("Could not locate line %u at file "RF_STR_PF_FMT,
                  m->loc.start.line,
                  RF_STR_PF_ARG(inpfile_name(input_file)));
        } else {
            fprintf(f, RF_STR_PF_FMT, RF_STR_PF_ARG(&line_str));
            print_location_marker(f, m->loc.start.col);
        }
        break;
    default: /* should never get here */
        assert(0);
        break;
    }
}

bool info_msg_get_formatted(struct info_msg *m, struct RFstringx *s,
                            struct inpfile *input_file)
{
    struct RFstring line_str;
    switch(m->type) {
    case MESSAGE_SEMANTIC_WARNING:
        rf_stringx_assignv(
            s,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));

        break;
    case MESSAGE_SYNTAX_WARNING:
        rf_stringx_assignv(
            s,
            INPLOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));

        break;
    case MESSAGE_SEMANTIC_ERROR:
        rf_stringx_assignv(
            s,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));
        break;
    case MESSAGE_SYNTAX_ERROR:
        rf_stringx_assignv(
            s,
            INPLOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            INPLOCATION_ARG(input_file, &m->loc),
            RF_STR_PF_ARG(&m->s));
        if (!inpfile_line(input_file, m->loc.start.line, &line_str)) {
            ERROR("Could not locate line %u at file "RF_STR_PF_FMT,
                  m->loc.start.line,
                  RF_STR_PF_ARG(inpfile_name(input_file)));
            return false;
        } else {
            rf_stringx_move_end(s);
            rf_stringx_assignv(s, RF_STR_PF_FMT"\n", RF_STR_PF_ARG(&line_str));
            rf_stringx_move_end(s);
            rf_stringx_assignv(s, LOCMARK_FMT, LOCMARK_ARG(m->loc.start.col));
        }
        break;
    default: /* should never get here */
        RF_ASSERT(0);
        return false;
        break;
    }

    return true;
}
