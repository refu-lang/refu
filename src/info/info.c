#include <info/info.h>

#include <Utils/bits.h>

#include <info/msg.h>
#include <compiler_args.h>
#include <inplocation.h>
#include <inpfile.h>


#define INFO_CTX_BUFF_SIZE 512 //TODO: move somewhere else

struct info_ctx *info_ctx_create(struct inpfile *f)
{
    struct info_ctx *ctx;

    RF_MALLOC(ctx, sizeof(*ctx), return NULL);
    rf_ilist_head_init(&ctx->msg_list);
    ctx->msg_num = 0;
    if (!rf_stringx_init_buff(&ctx->buff, INFO_CTX_BUFF_SIZE, "")) {
        free(ctx);
        RF_ERRNOMEM();
        return NULL;
    }
    ctx->syntax_error = false;
    ctx->file = f;

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

#if 0 // was used in the old INFO() message to judge if verbosity is high enough to print message. Find better way.
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
#endif


bool i_info_ctx_add_msg(struct info_ctx *ctx,
                        enum info_msg_type type,
                        const struct inplocation_mark *start,
                        const struct inplocation_mark *end,
                        const char *fmt,
                        ...)
{
    va_list args;
    struct info_msg *msg;

    va_start(args, fmt);
    msg = info_msg_create(type, start, end, fmt, args);
    va_end(args);

    if (!msg) {
        return false;
    }

    rf_ilist_add_tail(&ctx->msg_list, &msg->ln);
    ctx->msg_num++;
    return true;
}

void info_ctx_rem_messages(struct info_ctx *ctx, size_t num)
{
    struct info_msg *m;
    struct info_msg *tmp;
    size_t i = 0;
    rf_ilist_for_each_safe(&ctx->msg_list, m, tmp, ln) {
        if (ctx->msg_num - i <= num) {
            rf_ilist_delete_from(&ctx->msg_list, &m->ln);
            info_msg_destroy(m);
        }
        ++i;
    }
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

            info_msg_print(m, f, ctx->file);
            rf_ilist_delete_from(&ctx->msg_list, &m->ln);
            info_msg_destroy(m);
        }
    }
}

i_INLINE_INS void info_ctx_inject_input_file(struct info_ctx *ctx, struct inpfile *f);

bool info_ctx_get_messages_fmt(struct info_ctx *ctx,
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
            if (!info_msg_get_formatted(m, str, ctx->file)) {
                return false;
            }
            rf_stringx_move_end(str);
        }
    }
    rf_stringx_reset(str);
    return true;
}


void info_ctx_get_iter(struct info_ctx *ctx,
                       enum info_msg_type types,
                       struct info_ctx_msg_iterator *iter)
{
    iter->msg_types = types;
    iter->start = &ctx->msg_list.n;
    iter->curr = &ctx->msg_list.n;
}

struct info_msg *info_ctx_msg_iterator_next(struct info_ctx_msg_iterator *iter)
{
    struct info_msg *msg;

    // if empty
    if (iter->curr->next == iter->start) {
        return NULL;
    }

    do {
        iter->curr = iter->curr->next;
        msg = rf_ilist_entry(iter->curr, struct info_msg, ln);
        if (msg->type & iter->msg_types) {
            return msg;
        }
    } while (iter->curr->next != iter->start);
    return NULL;
}
