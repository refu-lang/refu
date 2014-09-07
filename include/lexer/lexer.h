#ifndef LFR_LEXER_H
#define LFR_LEXER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <lexer/tokens.h>
#include <ast/location.h>


struct parser_file;

struct token {
    enum token_type type;
    struct ast_location loc;
    union {
        struct RFstring string;
        int numeric;
    }value;
};

struct lexer {
    struct {darray(struct token);} tokens;
    struct {darray(int);} indices;
    struct parser_file *file;
};


bool lexer_init(struct lexer *l);
void lexer_deinit(struct lexer *l);

bool lexer_scan(struct lexer *l, struct parser_file *f);
struct token *lexer_next_token(struct lexer *l);
void lexer_save_pos(struct lexer *l);
void lexer_rollback(struct lexer *l);
void lexer_okay(struct lexer *l);


#endif
