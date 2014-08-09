#include <info/msg.h>

#include <Utils/memory.h>

struct info_msg *info_msg_create(enum info_msg_type type,
                                 struct ast_location *loc,
                                 const char *fmt, ...)
{
    struct info_msg *ret;
    va_list args;
    RF_MALLOC(ret, sizeof(*ret), NULL);

    va_start(args, fmt);
    if (!rf_string_initv(&ret->s, fmt, args)) {
        va_end(args);    
        return NULL;
    }
    va_end(args);    

    ret->type = type;
    ast_location_copy(&ret->loc, loc);

    return ret;
}

struct info_msg *info_msg_createv(enum info_msg_type type,
                                  struct ast_location *loc,
                                  const char *fmt, va_list va)
{
    return NULL;//TODO
}

void info_msg_destroy(struct info_msg *m)
{
    rf_string_deinit(&m->s);
    free(m);
}
