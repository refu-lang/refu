#ifndef LFR_PARSER_COMMON_H
#define LFR_PARSER_COMMON_H

#include <Definitions/inline.h>
#include <stddef.h>
#include <stdbool.h>

struct inpfile;
struct lexer;
struct info_ctx;

enum parser_type {
    PARSER_AST,
    PARSER_RIR
};

struct parser_common {
    //! The type of the parser
    enum parser_type type;
    //! The input file representation
    struct inpfile *file;
    //! The lexer part of the parser
    struct lexer *lexer;
    //! Pointer to the common info context
    struct info_ctx *info;
    //! A pointer to the front_ctx that owns the parser.
    struct front_ctx *front;
};

i_INLINE_DECL void parser_common_init(
    struct parser_common *c,
    enum parser_type type,
    struct front_ctx *front,
    struct inpfile *f,
    struct lexer *lexer,
    struct info_ctx *info
)
{
    c->type = type;
    c->front = front;
    c->file = f;
    c->lexer = lexer;
    c->info = info;
}

i_INLINE_DECL void parser_common_deinit(struct parser_common *c)
{
    c->lexer = NULL;
    c->file = NULL;
    c->info = NULL;
    c->front = NULL;
}

/**
 * Destroy a parser from its common data
 *
 * @note: This will call the appropriate destruction function and also destroy
 *        the parser pointer that contains the common data
 */
void parser_destroy(struct parser_common *c);

bool parser_parse(struct parser_common *c);


/**
 * Get the lexer associated with this parser
 */
#define parser_lexer(i_pptr_) ((i_pptr_)->cmn.lexer)

/**
 * Get the front context associated with this parser
 */
#define parser_front(i_pptr_) ((i_pptr_)->cmn.front)


struct ast_node *parser_ast_get_root(struct parser_common *c);
struct ast_node *parser_ast_move_root(struct parser_common *c);
struct rir *parser_rir(struct parser_common *c);

/*
 * The following macros are dangerous and need typechecking before use.
 * The type of the argument must be either ast_parser or rir_parser. Anything
 * else will lead to ugly undefined behaviour
 */

/**
 * Get the parser's common data from any parser.
 */
#define parser_to_common(i_pptr_) ((struct parser_common*)i_pptr_)

#endif

