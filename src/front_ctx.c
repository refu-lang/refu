#include <front_ctx.h>

#include <info/info.h>
#include <lexer/lexer.h>
#include <parser/parser.h>

bool front_ctx_init(struct front_ctx *ctx,
                    const struct RFstring *filename)
{
    if (!inpfile_init(&ctx->file, filename)) {
        goto free_info;
    }

    ctx->info = info_ctx_create(&ctx->file);
    if (!ctx->info) {
        return false;
    }

    ctx->lexer = lexer_create(&ctx->file, ctx->info);
    if (!ctx->lexer) {
        goto free_file;
    }

    ctx->parser = parser_create(&ctx->file, ctx->lexer, ctx->info);
    if (!ctx->parser) {
        goto free_lexer;
    }

    return true;

free_lexer:
    lexer_destroy(ctx->lexer);
free_file:
    inpfile_deinit(&ctx->file);
free_info:
    info_ctx_destroy(ctx->info);
    return false;
}

struct front_ctx *front_ctx_create(const struct RFstring *filename)
{
    struct front_ctx *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!front_ctx_init(ret, filename)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void front_ctx_deinit(struct front_ctx *ctx)
{
    inpfile_deinit(&ctx->file);
    lexer_destroy(ctx->lexer);
    parser_destroy(ctx->parser);
    info_ctx_destroy(ctx->info);
}

void front_ctx_destroy(struct front_ctx *ctx)
{
    front_ctx_deinit(ctx);
    free(ctx);
}

bool front_ctx_process(struct front_ctx *ctx)
{
    if (!lexer_scan(ctx->lexer)) {
        return false;
    }

    if (parser_process_file(ctx->parser)) {
        return false;
    }

    return true;
}
