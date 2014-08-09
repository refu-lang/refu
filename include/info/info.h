#ifndef LFR_INFO_H
#define LFR_INFO_H

#include <RFintrusive_list.h>
#include <RFstring.h>

enum info_msg_type {
    MESSAGE_ANY = 0x1,
    MESSAGE_SEMANTIC_WARNING = 0x2,
    MESSAGE_PARSING_WARNING = 0x4,
    MESSAGE_SEMANTIC_ERROR = 0x8,
    MESSAGE_PARSING_ERROR = 0x10
};

struct ast_location;

struct info_ctx {
    RFilist_head msg_list;
    int verbose_level;
    struct RFstringx buff;
};


struct info_ctx *info_ctx_create();
void info_print_cond(int vlevel, const char *fmt, ...);

bool i_info_ctx_add_msg(struct info_ctx *ctx,
                        enum info_msg_type type,
                        struct ast_location *loc,
                        const char *fmt,
                        ...);

bool info_ctx_has(struct info_ctx *ctx);
void info_ctx_flush(struct info_ctx *ctx, FILE *f, int type);

/* simple printing related function wrappers */

#define ERROR(...)                              \
    do {                                        \
        printf("refu: [error] "__VA_ARGS__);    \
        printf("\n");                           \
    } while(0)

#define INFO(level_, ...)                                     \
    do {                                                      \
        info_print_cond(level_, "refu: [info] "__VA_ARGS__);  \
        printf("\n");                                         \
    } while(0)

#define WARN(...)                               \
    do {                                        \
        printf("refu: [warning] "__VA_ARGS__);  \
        printf("\n");                           \
    } while(0)

#endif
