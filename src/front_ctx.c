#include <front_ctx.h>

#include <utils/common_strings.h>
#include <module.h>
#include <compiler_args.h>
#include <compiler.h>
#include <info/info.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <ir/parser/rirparser.h>
#include <analyzer/analyzer.h>
#include <serializer/serializer.h>

static bool front_ctx_init(
    struct front_ctx *ctx,
    const struct compiler_args *args,
    enum rir_pos pos,
    const struct RFstring *input_file_name,
    const struct RFstring *file_contents
)
{
    RF_STRUCT_ZERO(ctx);
    // sanity check on the argument constraints
    RF_ASSERT(
        args || (pos == RIRPOS_PARSE || pos == RIRPOS_AST),
        "Either arguments or a valid codepath pos should have been provided"
    );


    // determine the file contents
    ctx->file = file_contents
        ? inpfile_create_from_string(input_file_name, file_contents)
        : inpfile_create(input_file_name);
    if (!ctx->file) {
        goto err;
    }

    // determine the code path
    enum rir_pos codepath = pos == RIRPOS_NONE
        ? compiler_arg_input_is_rir(args) ? RIRPOS_PARSE : RIRPOS_AST
        : pos;

    ctx->info = info_ctx_create(ctx->file);
    if (!ctx->info) {
        goto free_file;
    }

    ctx->lexer = lexer_create(ctx->file, ctx->info, codepath);
    if (!ctx->lexer) {
        goto free_info;
    }

    void *parser;
    if (codepath == RIRPOS_PARSE) {
        parser = rir_parser_create(ctx, ctx->file, ctx->lexer, ctx->info);
    } else {
        parser = ast_parser_create(ctx->file, ctx->lexer, ctx->info, ctx);
    }
    if (!parser) {
        goto free_lexer;
    }
    ctx->parser = parser_to_common(parser);

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

struct front_ctx *front_ctx_create(
    const struct compiler_args *args,
    enum rir_pos pos,
    const struct RFstring *input_file_name,
    const struct RFstring *file_contents
)
{
    struct front_ctx *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!front_ctx_init(ret, args, pos, input_file_name, file_contents)) {
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

bool front_ctx_make_main(struct front_ctx *f, struct ast_node *n, struct rir *rir)
{
    if (!compiler_set_main(f)) {
        return false;
    }
    return module_create(n, rir, f);
}

bool front_ctx_parse(struct front_ctx *ctx)
{
    if (!lexer_scan(ctx->lexer)) {
        return false;
    }

    if (!parser_parse(ctx->parser)) {
        return false;
    }

    switch (ctx->parser->type) {
    case PARSER_AST:
        // the root should no longer be owned by the parser at this point
        ctx->root = parser_ast_move_root(ctx->parser);
        // root will never go into analyzer pass1 so set the state properly here
        ctx->root->state = AST_NODE_STATE_ANALYZER_PASS1;
        // finally make sure that the root's symbol table is initialized
        return root_symbol_table_init(&ctx->root->root.st);
    case PARSER_RIR:
        return true;
    default:
        RF_CRITICAL_FAIL("Illegal parser type");
        break;
    }
    return false;
}
