#ifndef LFR_FRONT_CTX_H
#define LFR_FRONT_CTX_H

#include <inpfile.h>

struct info_ctx;
struct lexer;
struct parser;
struct analyzer;

/**
 * The front end context
 * Defines and manages the front end pipeline an input file
 * goes through until the intermediate representation.
 */
struct front_ctx {
    /* Owned objects */
    struct inpfile file;
    struct lexer *lexer;
    struct parser *parser;
    struct analyzer *analyzer;
    struct info_ctx *info;
};

bool front_ctx_init(struct front_ctx *ctx,
                    const struct RFstring *filename);
struct front_ctx *front_ctx_create(const struct RFstring *filename);

void front_ctx_deinit(struct front_ctx *ctx);
void front_ctx_destroy(struct front_ctx *ctx);


bool front_ctx_process(struct front_ctx *ctx);

#endif
