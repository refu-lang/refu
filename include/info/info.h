#ifndef LFR_INFO_H
#define LFR_INFO_H

#include <RFintrusive_list.h>
#include <RFstring.h>

enum info_msg_type {
    MESSAGE_ANY = 0x1,
    MESSAGE_SEMANTIC_WARNING = 0x2,
    MESSAGE_SYNTAX_WARNING = 0x4,
    MESSAGE_SEMANTIC_ERROR = 0x8,
    MESSAGE_SYNTAX_ERROR = 0x10
};

struct inplocation;

struct info_ctx {
    RFilist_head msg_list;
    int verbose_level;
    struct RFstringx buff;
    bool syntax_error; /* maybe to avoid searching the whole list? */

    // a pointer to the file all the info messages will refer to (not owned)
    struct inpfile *file;
};


struct info_ctx *info_ctx_create(struct inpfile *file);
void info_ctx_destroy(struct info_ctx *ctx);

void info_print_cond(int vlevel, const char *fmt, ...);

bool i_info_ctx_add_msg(struct info_ctx *ctx,
                        enum info_msg_type type,
                        struct inplocation *loc,
                        const char *fmt,
                        ...);

bool info_ctx_has(struct info_ctx *ctx, enum info_msg_type type);
void info_ctx_flush(struct info_ctx *ctx, FILE *f, int type);
void info_ctx_print(struct info_ctx *ctx, unsigned int index);

/**
 * Gets all messages of a certain type, properly formatted and returns them
 * in the given RFstringx
 * @param ctx        The info context to work with
 * @param type       The type of messages to retrieve
 * @param str        The string in which to return
 * @return           True/false for success/failure
*/
bool info_ctx_get_messages_fmt(struct info_ctx *ctx,
                               enum info_msg_type type,
                               struct RFstringx *str);

/**
 * An iterator allowing iteration over the messages of an info_ctx
 */
struct info_ctx_msg_iterator {
    enum info_msg_type msg_types;
    struct RFilist_node *start;
    struct RFilist_node *next;
};
void info_ctx_get_iter(struct info_ctx *ctx,
                       enum info_msg_type types,
                       struct info_ctx_msg_iterator *iter);
struct info_msg *info_ctx_msg_iterator_next(struct info_ctx_msg_iterator *it);

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
