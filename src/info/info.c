#include <info/info.h>

#include <Utils/bits.h>

#include <info/msg.h>
#include <compiler_args.h>
#include <ast/location.h>
#include <parser/file.h>


#define INFO_CTX_BUFF_SIZE 512 //TODO: move somewhere else

bool i_info_ctx_add_msg(struct info_ctx *ctx,
                        enum info_msg_type type,
                        struct ast_location *loc,
                        const char *fmt,
                        ...)
{
    va_list args;
    struct info_msg *msg;

    va_start(args, fmt);
    msg = info_msg_create(type, loc, fmt, args);
    va_end(args);

    if (!msg) {
        return false;
    }

    rf_ilist_add_tail(&ctx->msg_list, &msg->ln);
    return true;
}

bool info_ctx_has(struct info_ctx *ctx, enum info_msg_type type)
{
    struct info_msg *m;
    if (type == MESSAGE_ANY) {
        return !rf_ilist_is_empty(&ctx->msg_list);
    }

    rf_ilist_for_each(&ctx->msg_list, m, ln) {
        if (RF_BITFLAG_ON(type, m->type)) {
            return true;
        }
    }
    return false;
}

void info_ctx_flush(struct info_ctx *ctx, FILE *f, int type)
{
    struct info_msg *m;
    struct info_msg *tmp;

    rf_ilist_for_each_safe(&ctx->msg_list, m, tmp, ln) {
        if (RF_BITFLAG_ON(type, MESSAGE_ANY) ||
            RF_BITFLAG_ON(type, m->type)) {

            info_msg_print(m, f);
            rf_ilist_delete_from(&ctx->msg_list, &m->ln);
            info_msg_destroy(m);
        }
    }
}

void info_print_cond(int vlevel, const char *fmt, ...)
{
    struct compiler_args *cargs = compiler_args_get();
    if (cargs->verbose_level >= vlevel) {
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

bool info_ctx_get(struct info_ctx *ctx,
                  enum info_msg_type type,
                  struct RFstringx *str)
{
    struct info_msg *m;
    if (rf_ilist_is_empty(&ctx->msg_list)) {
        return false;
    }
    rf_ilist_for_each(&ctx->msg_list, m, ln) {
        if (RF_BITFLAG_ON(type, MESSAGE_ANY) ||
            RF_BITFLAG_ON(type, m->type)) {
            if (!info_msg_get_formatted(m, str)) {
                return false;
            }
            rf_stringx_move_end(str);
        }
    }
    rf_stringx_reset(str);
    return true;
}


struct info_ctx *info_ctx_create()
{
    struct info_ctx *ctx;

    RF_MALLOC(ctx, sizeof(*ctx), return NULL);
    rf_ilist_head_init(&ctx->msg_list);
    if (!rf_stringx_init_buff(&ctx->buff, INFO_CTX_BUFF_SIZE, "")) {
        free(ctx);
        return NULL;
    }
    ctx->syntax_error = false;

    return ctx;
}

void info_ctx_destroy(struct info_ctx *ctx)
{
    struct info_msg *m;
    struct info_msg *tmp;

    rf_ilist_for_each_safe(&ctx->msg_list, m, tmp, ln) {
        info_msg_destroy(m);
    }
    rf_stringx_deinit(&ctx->buff);
    free(ctx);
}
