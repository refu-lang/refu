#ifndef LFR_FRONT_CTX_H
#define LFR_FRONT_CTX_H

#include <inpfile.h>
#include <analyzer/analyzer.h>
#include <RFintrusive_list.h>

struct info_ctx;
struct lexer;
struct parser;
struct compiler_args;

/**
 * The front end context
 * Defines and manages the front end pipeline an input file
 * goes through until the intermediate representation.
 */
struct front_ctx {
    /* Owned objects */
    struct inpfile *file;
    struct lexer *lexer;
    struct parser *parser;
    struct analyzer *analyzer;
    struct info_ctx *info;
    /* Control for adding to compiler object's linked list */
    struct RFilist_node ln;
};

bool front_ctx_init(struct front_ctx *ctx,
                    const struct compiler_args *args,
                    const struct RFstring *input_file);
struct front_ctx *front_ctx_create(const struct compiler_args *args,
                                   const struct RFstring *input_file);

void front_ctx_deinit(struct front_ctx *ctx);
void front_ctx_destroy(struct front_ctx *ctx);

struct analyzer *front_ctx_process(struct front_ctx *ctx);

/* -- some convenience setters/getters --*/
i_INLINE_DECL void front_ctx_set_warn_on_implicit_conversions(struct front_ctx *ctx,
                                                              bool v)
{
    ctx->analyzer->warn_on_implicit_conversions = v;
}

#endif
