#include <info/msg.h>

#include <Utils/memory.h>
#include <parser/file.h>

#define INFO_WARNING_STR "warning"
#define INFO_ERROR_STR "error"

inline static void print_location_marker(FILE *f, unsigned int col_pos)
{
    printf("%*c\n", col_pos, '^');
}

struct info_msg *info_msg_create(enum info_msg_type type,
                                 struct ast_location *loc,
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
    ast_location_copy(&ret->loc, loc);

    return ret;
}

void info_msg_destroy(struct info_msg *m)
{
    rf_string_deinit(&m->s);
    free(m);
}

void info_msg_print(struct info_msg *m, FILE *f)
{
    struct RFstring line_str;
    switch(m->type) {
    case MESSAGE_SEMANTIC_WARNING:
        fprintf(
            f,
            AST_LOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            AST_LOCATION_ARG(&m->loc), 
            RF_STR_PF_ARG(&m->s));
                            
        break;
    case MESSAGE_SYNTAX_WARNING:
        fprintf(
            f,
            AST_LOCATION_FMT" "INFO_WARNING_STR": "RF_STR_PF_FMT"\n",
            AST_LOCATION_ARG(&m->loc), 
            RF_STR_PF_ARG(&m->s));

        break;
    case MESSAGE_SEMANTIC_ERROR:
        fprintf(
            f,
            AST_LOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            AST_LOCATION_ARG(&m->loc), 
            RF_STR_PF_ARG(&m->s));
        break;
    case MESSAGE_SYNTAX_ERROR:
        fprintf(
            f,
            AST_LOCATION_FMT" "INFO_ERROR_STR": "RF_STR_PF_FMT"\n",
            AST_LOCATION_ARG(&m->loc), 
            RF_STR_PF_ARG(&m->s));
        if (!parser_file_line(m->loc.file, m->loc.start_line, &line_str)) {
            ERROR("Could not locate line %u at file "RF_STR_PF_FMT,
                  m->loc.start_line,
                  RF_STR_PF_ARG(ast_location_filename(&m->loc)));
        } else {
            fprintf(f, RF_STR_PF_FMT, RF_STR_PF_ARG(&line_str));
            print_location_marker(f, m->loc.start_col);
        }
        break;
    default: /* should never get here */
        assert(0);
        break;
    }
}
