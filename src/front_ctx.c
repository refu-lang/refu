#include <front_ctx.h>

#include <module.h>
#include <compiler_args.h>
#include <info/info.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <analyzer/analyzer.h>
#include <serializer/serializer.h>

static bool front_ctx_init(struct front_ctx *ctx,
                           const struct compiler_args *args,
                           const struct RFstring *input_file_name,
                           const struct RFstring *file_contents)
{
    RF_STRUCT_ZERO(ctx);
    ctx->file = file_contents
        ? inpfile_create_from_string(input_file_name, file_contents)
        : inpfile_create(input_file_name);
    if (!ctx->file) {
        goto err;
    }

    ctx->info = info_ctx_create(ctx->file);
    if (!ctx->info) {
        goto free_file;
    }

    ctx->lexer = lexer_create(ctx->file, ctx->info);
    if (!ctx->lexer) {
        goto free_info;
    }

    ctx->parser = parser_create(ctx->file, ctx->lexer, ctx->info, ctx);
    if (!ctx->parser) {
        goto free_lexer;
    }

    ctx->is_main = false;

    return true;

free_lexer:
    lexer_destroy(ctx->lexer);
free_info:
    info_ctx_destroy(ctx->info);
free_file:
    inpfile_destroy(ctx->file);
err:
    return false;
}

struct front_ctx *front_ctx_create(const struct compiler_args *args,
                                   const struct RFstring *input_file)
{
    struct front_ctx *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!front_ctx_init(ret, args, input_file, NULL)) {
        free(ret);
        return NULL;
    }
    return ret;
}

struct front_ctx *front_ctx_create_from_source(const struct compiler_args *args,
                                               const struct RFstring *file_name,
                                               const struct RFstring *source)
{
    struct front_ctx *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!front_ctx_init(ret, args, file_name, source)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void front_ctx_deinit(struct front_ctx *ctx)
{
    if (ctx->root) {
        ast_node_destroy(ctx->root);
    }
    inpfile_destroy(ctx->file);
    lexer_destroy(ctx->lexer);
    parser_destroy(ctx->parser);
    info_ctx_destroy(ctx->info);
}

void front_ctx_destroy(struct front_ctx *ctx)
{
    front_ctx_deinit(ctx);
    free(ctx);
}

struct RFstring *front_ctx_filename(const struct front_ctx *f)
{
    return inpfile_name(f->file);
}

bool front_ctx_parse(struct front_ctx *ctx)
{
    if (!lexer_scan(ctx->lexer)) {
        return false;
    }

    if (!parser_process_file(ctx->parser)) {
        return false;
    }
    // the root should no longer be owned by the parser at this point
    ctx->root = ctx->parser->root;
    ctx->parser->root = 0;
    // root will never go into analyzer pass1 so set the state properly here
    ctx->root->state = AST_NODE_STATE_ANALYZER_PASS1;
    // finally make sure that the root's symbol table is initialized
    return root_symbol_table_init(&ctx->root->root.st);
}
