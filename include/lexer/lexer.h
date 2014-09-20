#ifndef LFR_LEXER_H
#define LFR_LEXER_H

#include <stdbool.h>

#include <Data_Structures/darray.h>

#include <lexer/tokens.h>
#include <inplocation.h>
#include <ast/identifier.h>


struct inpfile;

/* A struct to group an identifier token with memory ownership semantics */
struct tok_identifier {
    struct ast_node *id;
    bool owned_by_lexer;
};

struct token {
    enum token_type type;
    struct inplocation location;
    union {
        struct tok_identifier identifier;
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

// gets the identifier from a token. Use only after a tok->type check
i_INLINE_DECL struct ast_node *token_get_identifier(struct token *tok)
{
    tok->value.identifier.owned_by_lexer = false;
    return tok->value.identifier.id;
}


struct lexer {
    struct {darray(struct token);} tokens;
    struct {darray(int);} indices;
    unsigned int tok_index;
    struct inpfile *file;
    struct info_ctx *info;
    //! Denotes that the lexer has reached the end of its input
    bool at_eof;
};


bool lexer_init(struct lexer *l, struct inpfile *f, struct info_ctx *info);
struct lexer *lexer_create(struct inpfile *f, struct info_ctx *info);
void lexer_deinit(struct lexer *l);
void lexer_destroy(struct lexer *l);

bool lexer_scan(struct lexer *l);

struct token *lexer_next_token(struct lexer *l);
struct token *lexer_lookahead(struct lexer *l, unsigned int num);
struct token *lexer_last_token_valid(struct lexer *l);

i_INLINE_DECL struct inplocation *lexer_last_token_location(struct lexer *l)
{
    return &(lexer_last_token_valid(l))->location;
}

i_INLINE_DECL struct inplocation_mark *lexer_last_token_start(struct lexer *l)
{
    return &(lexer_last_token_valid(l))->location.start;
}

i_INLINE_DECL struct inplocation_mark *lexer_last_token_end(struct lexer *l)
{
    return &(lexer_last_token_valid(l))->location.end;
}

void lexer_push(struct lexer *l);
void lexer_pop(struct lexer *l);
void lexer_rollback(struct lexer *l);

#endif
