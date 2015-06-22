#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

struct info_ctx;
struct lexer;
struct inpfile;
struct nodes_arr;

struct parser {
    struct info_ctx *info;
    struct lexer *lexer;
    struct inpfile *file;
    struct ast_node *root;
    //! A pointer to the modules array. Not owned by the parser.
    struct nodes_arr *modules_array;
    bool have_syntax_err;
};


bool parser_init(struct parser *p,
                 struct inpfile *f,
                 struct lexer *lex,
                 struct info_ctx *info);
struct parser *parser_create(struct inpfile *f,
                             struct lexer *lex,
                             struct info_ctx *info);
void parser_deinit(struct parser *p);
void parser_destroy(struct parser *p);

bool parser_process_file(struct parser *p, struct nodes_arr *modules_array);
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

i_INLINE_DECL struct ast_node *parser_yield_ast_root(struct parser *parser)
{
    struct ast_node *root;
    root = parser->root;
    parser->root = NULL;
    return root;
}

i_INLINE_DECL void parser_inject_input_file(struct parser *p, struct inpfile *f)
{
    p->file = f;
}

/**
 * Mark all children of node @a n as finalized after parsing
 */
void parser_finalize_parsing(struct ast_node *n);

#endif
