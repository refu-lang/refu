#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/string.h>

#include <parser/parser_common.h>

struct info_ctx;
struct lexer;
struct inpfile;
struct front_ctx;

struct ast_parser {
    //! The parser common data. Should always be first. Some behaviour relies on that.
    struct parser_common cmn;
    struct ast_node *root;
    bool have_syntax_err;
};

i_INLINE_DECL struct ast_parser *parser_common_to_astparser(const struct parser_common* c)
{
    RF_ASSERT(c->type == PARSER_AST, "Expected AST parser");
    return container_of(c, struct ast_parser, cmn);
}


bool ast_parser_init(
    struct ast_parser *p,
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *info,
    struct front_ctx *front
);
struct ast_parser *ast_parser_create(
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *info,
    struct front_ctx *front
);
void ast_parser_deinit(struct ast_parser *p);
void ast_parser_destroy(struct ast_parser *p);


/**
 * Performs the scanning and parsing stage on a file
 */
bool ast_parser_parse_file(struct ast_parser *p);
/**
 * Mark all children of node @a n as finalized after parsing and 
 * checks for a main function to see if this should be the main module
 */
bool ast_parser_finalize_parsing(struct ast_parser *p);
/**
 * Pre-analysis stage for a file. Scanning, parsing and finalization
 * This is the main function for parsing.
 */
bool ast_parser_process_file(struct ast_parser *p);



i_INLINE_DECL void ast_parser_set_syntax_error(struct ast_parser *parser)
{
    parser->have_syntax_err = true;
}

i_INLINE_DECL bool ast_parser_has_syntax_error(struct ast_parser *parser)
{
    return parser->have_syntax_err;
}

i_INLINE_DECL bool ast_parser_has_syntax_error_reset(struct ast_parser *parser)
{
    bool ret = parser->have_syntax_err;
    parser->have_syntax_err = false;
    return ret;
}

/**
 * Acts just like info_ctx_rollback() except it also resets the has_syntax_error
 * variable.
 *
 * TODO: Make this smarter, get rid of the has_syntax_error logic and just use the
 * rollback mechanism if possible
 */
void ast_parser_info_rollback(struct ast_parser *parser);


#endif
