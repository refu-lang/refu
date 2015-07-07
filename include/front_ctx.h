#ifndef LFR_FRONT_CTX_H
#define LFR_FRONT_CTX_H

#include <inpfile.h>
#include <analyzer/analyzer.h>
#include <RFintrusive_list.h>
#include <module.h>

struct info_ctx;
struct lexer;
struct parser;
struct compiler_args;
struct ast_node;


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
    struct info_ctx *info;
    //! Denotes whether this file is the starting point of our project, hence the main module
    bool is_main;
    //! Pointer to the root AST node for the file, valid only after parsing is finalized
    struct ast_node *root;
    /* Control for adding to compiler object's linked list */
    struct RFilist_node ln;
};

struct front_ctx *front_ctx_create(const struct compiler_args *args,
                                   const struct RFstring *file_name);
struct front_ctx *front_ctx_create_from_source(const struct compiler_args *args,
                                               const struct RFstring *file_name,
                                               const struct RFstring *src);

void front_ctx_deinit(struct front_ctx *ctx);
void front_ctx_destroy(struct front_ctx *ctx);

struct RFstring *front_ctx_filename(const struct front_ctx *f);

/**
 * Scan and parse the file of a front_ctx
 */
bool front_ctx_parse(struct front_ctx *ctx);

#endif
