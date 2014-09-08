#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

struct info_ctx;
struct lexer;
struct inpfile;

struct parser {
    struct info_ctx *info;
    struct lexer *lexer;
    struct inpfile *file;
    struct ast_node *root;
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

bool parser_process_file(struct parser *parser);
void parser_flush_messages(struct parser *parser);
#endif
