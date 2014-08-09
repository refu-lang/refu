#include <info/info.h>

#include <Utils/bits.h>

#include <info/msg.h>
#include <compiler_args.h>
#include <ast/location.h>
#include <parser/file.h>


#define INFO_CTX_BUFF_SIZE 512 //TODO: move somewhere else

#define INFO_WARNING_STR "warning"
#define INFO_ERROR_STR "error"

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

bool info_ctx_has(struct info_ctx *ctx)
{
    return !rf_ilist_is_empty(&ctx->msg_list);
}


void info_ctx_flush(struct info_ctx *ctx, FILE *f, int type)
{
    struct info_msg *m;
    struct info_msg *tmp;

    rf_ilist_for_each_safe(&ctx->msg_list, m, tmp, ln)
    {
        if (RF_BITFLAG_ON(type, MESSAGE_ANY) ||
            RF_BITFLAG_ON(type, m->type)) {

                switch(m->type) {
                    case MESSAGE_SEMANTIC_WARNING:
                        fprintf(
                            f,
                            AST_LOCATION_FMT" "INFO_WARNING_STR":"RF_STR_PF_FMT,
                            AST_LOCATION_ARG(&m->loc), 
                            RF_STR_PF_ARG(&m->s));
                            
                        break;
                    case MESSAGE_PARSING_WARNING:
                        fprintf(
                            f,
                            AST_LOCATION_FMT" "INFO_WARNING_STR":"RF_STR_PF_FMT,
                            AST_LOCATION_ARG(&m->loc), 
                            RF_STR_PF_ARG(&m->s));

                        break;
                    case MESSAGE_SEMANTIC_ERROR:
                        fprintf(
                            f,
                            AST_LOCATION_FMT" "INFO_ERROR_STR":"RF_STR_PF_FMT,
                            AST_LOCATION_ARG(&m->loc), 
                            RF_STR_PF_ARG(&m->s));
                        break;
                    case MESSAGE_PARSING_ERROR:
                        fprintf(
                            f,
                            AST_LOCATION_FMT" "INFO_ERROR_STR":"RF_STR_PF_FMT,
                            AST_LOCATION_ARG(&m->loc), 
                            RF_STR_PF_ARG(&m->s));

                        break;
                    default: /* should never get here */
                        assert(0);
                        break;
                }

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
        printf(fmt, args);
        va_end(args);
    }
}


struct info_ctx *info_ctx_create()
{
    struct info_ctx *ctx;

    RF_MALLOC(ctx, sizeof(*ctx), NULL);
    rf_ilist_head_init(&ctx->msg_list);
    if (!rf_stringx_init_buff(&ctx->buff, INFO_CTX_BUFF_SIZE, "")) {
        free(ctx);
        return NULL;
    }

    return ctx;
}
