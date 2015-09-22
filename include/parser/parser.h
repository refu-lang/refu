#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

struct info_ctx;
struct lexer;
struct inpfile;
struct front_ctx;

struct parser {
    //! A pointer to the front_ctx that owns the parser. Needed only by the
    //! parsing of a module function. Maybe pass only as argument?
    struct front_ctx *front;
    struct info_ctx *info;
    struct lexer *lexer;
    struct inpfile *file;
    struct ast_node *root;
    bool have_syntax_err;
};


bool parser_init(struct parser *p,
                 struct inpfile *f,
                 struct lexer *lex,
                 struct info_ctx *info,
                 struct front_ctx *front);
struct parser *parser_create(struct inpfile *f,
                             struct lexer *lex,
                             struct info_ctx *info,
                             struct front_ctx *front);
void parser_deinit(struct parser *p);
void parser_destroy(struct parser *p);


/**
 * Performs the scanning and parsing stage on a file
 */
bool parser_parse_file(struct parser *p);
/**
 * Mark all children of node @a n as finalized after parsing and 
 * checks for a main function to see if this should be the main module
 */
bool parser_finalize_parsing(struct parser *p);
/**
 * Pre-analysis stage for a file. Scanning, parsing and finalization
 * This is the main function for parsing.
 */
bool parser_process_file(struct parser *p);

void parser_flush_messages(struct parser *parser);

i_INLINE_DECL void parser_set_syntax_error(struct parser *parser)
{
    parser->have_syntax_err = true;
}

i_INLINE_DECL bool parser_has_syntax_error(struct parser *parser)
{
    return parser->have_syntax_err;
}

i_INLINE_DECL bool parser_has_syntax_error_reset(struct parser *parser)
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
void parser_info_rollback(struct parser *parser);

i_INLINE_DECL void parser_inject_input_file(struct parser *p, struct inpfile *f)
{
    p->file = f;
}

#endif
