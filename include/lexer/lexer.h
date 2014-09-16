#ifndef LFR_LEXER_H
#define LFR_LEXER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <lexer/tokens.h>
#include <inplocation.h>
#include <ast/identifier.h>


struct inpfile;

struct token {
    enum token_type type;
    struct inplocation location;
    union {
        struct ast_node *identifier;
        int numeric;
    }value;
};

i_INLINE_DECL struct inplocation *token_get_loc(struct token *tok)
{
    return &tok->location;
}
i_INLINE_DECL struct inplocation_mark *token_get_start(struct token *tok)
{
    return &tok->location.start;
}
i_INLINE_DECL struct inplocation_mark *token_get_end(struct token *tok)
{
    return &tok->location.end;
}


struct lexer {
    struct {darray(struct token);} tokens;
    struct {darray(int);} indices;
    unsigned int tok_index;
    struct inpfile *file;
    struct info_ctx *info;
    //! Denotes whether or not the lexer should free
    //! the memory of identifier tokens at deinitialization
    bool own_identifier_ptrs;
    //! Denotes that the lexer has reached the end of its input
    bool at_eof;
};


bool lexer_init(struct lexer *l, struct inpfile *f, struct info_ctx *info);
struct lexer *lexer_create(struct inpfile *f, struct info_ctx *info);
void lexer_deinit(struct lexer *l);
void lexer_destroy(struct lexer *l);

bool lexer_scan(struct lexer *l);

struct token *lexer_next_token(struct lexer *l);
struct token *lexer_lookeahead(struct lexer *l, unsigned int num);
struct token *lexer_last_token_valid(struct lexer *l);
i_INLINE_DECL struct inplocation *lexer_last_token_location(struct lexer *l)
{
    return &(lexer_last_token_valid(l))->location;
}

void lexer_push(struct lexer *l);
void lexer_pop(struct lexer *l);
void lexer_rollback(struct lexer *l);

i_INLINE_DECL void lexer_renounce_own_identifiers(struct lexer *l)
{
    l->own_identifier_ptrs = false;
}
#endif
