#include <lexer/lexer.h>
#include <inpfile.h>
#include <ast/ast.h>

#include "tokens_htable.h" /* include the gperf generated hash table */
#include "common.h"


i_INLINE_INS struct inplocation *token_get_loc(struct token *tok);
i_INLINE_INS struct inplocation_mark *token_get_start(struct token *tok);
i_INLINE_INS struct inplocation_mark *token_get_end(struct token *tok);
i_INLINE_INS struct ast_node *token_get_identifier(struct token *tok);

static inline bool token_init(struct token *t,
                              enum token_type type,
                              struct inpfile *f,
                              char *sp, char *ep)
{
    t->type = type;
    if (!inplocation_init(&t->location, f, sp, ep)) {
        return false;
    }
    return true;
}

static inline bool token_init_identifier(struct token *t,
                                         struct inpfile *f,
                                         char *sp, char *ep)
{
    if (!token_init(t, TOKEN_IDENTIFIER, f, sp, ep)) {
        return false;
    }
    t->value.identifier.id = ast_identifier_create(&t->location);
    t->value.identifier.owned_by_lexer = true;
    if (!t->value.identifier.id) {
        return false;
    }
    return true;
}

static inline bool token_init_numeric(struct token *t,
                                      struct inpfile *f,
                                      char *sp, char *ep)
{
    struct RFstring temp;
    if (!token_init(t, TOKEN_NUMERIC, f, sp, ep)) {
        return false;
    }
    RF_STRING_SHALLOW_INIT(&temp, sp, ep - sp + 1);
    if (!rf_string_to_int(&temp, &t->value.numeric)) {
        return false;
    }
    return true;
}

bool lexer_init(struct lexer *l, struct inpfile *f, struct info_ctx *info)
{
    darray_init(l->tokens);
    darray_init(l->indices);
    l->tok_index = 0;
    l->file = f;
    l->info = info;
    l->at_eof = false;
    return true;
}

struct lexer *lexer_create(struct inpfile *f, struct info_ctx *info)
{
    struct lexer *ret;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    if (!lexer_init(ret, f, info)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void lexer_deinit(struct lexer *l)
{
    struct token *tok;

    darray_foreach(tok, l->tokens) {
        if (tok->type == TOKEN_IDENTIFIER &&
            tok->value.identifier.owned_by_lexer) {

            ast_node_destroy(tok->value.identifier.id);
        }
    }

    darray_free(l->tokens);
    darray_free(l->indices);
}

void lexer_destroy(struct lexer *l)
{
    lexer_deinit(l);
    free(l);
}

// get the last token scanned by the lexer
static struct token *lexer_get_top_token(struct lexer *l)
{
    RF_ASSERT(!darray_empty(l->tokens));
    return &darray_top(l->tokens);
}

static bool lexer_add_token(struct lexer *l, enum token_type type,
                            char *sp, char* ep)
{
    darray_resize(l->tokens, l->tokens.size + 1);
    if (!token_init(&darray_top(l->tokens), type, l->file, sp, ep)) {
        return false;
    }

    return true;
}

static bool lexer_add_token_identifier(struct lexer *l,
                                       char *sp, char* ep)
{
    darray_resize(l->tokens, l->tokens.size + 1);
    if (!token_init_identifier(&darray_top(l->tokens), l->file, sp, ep)) {
        return false;
    }

    return true;
}

static bool lexer_add_token_numeric(struct lexer *l,
                                    char *sp, char* ep)
{
    darray_resize(l->tokens, l->tokens.size + 1);
    if (!token_init_numeric(&darray_top(l->tokens), l->file, sp, ep)) {
        return false;
    }

    return true;
}

#define COND_IDENTIFIER_BEGIN(p_)               \
    (((p_) >= 'A' && (p_) <= 'Z') ||            \
     ((p_) >= 'a' && (p_) <= 'z'))

#define COND_IDENTIFIER(p_)                     \
    (COND_IDENTIFIER_BEGIN(p_) ||               \
     ((p_) >= '0' && (p_) <= '9'))

static bool lexer_get_identifier(struct lexer *l, char *p,
                                 char *lim, char **ret_p)
{
    const struct internal_token *itoken;
    char *sp = p;
    while (p < lim) {
        if (COND_IDENTIFIER(*(p+1))) {
            p ++;
        } else {
            break;
        }
    }

    // check if it's a keyword
    itoken = lexer_lexeme_is_token(sp, p - sp + 1);
    if (itoken) {
        if (!lexer_add_token(l, itoken->type, sp, p)) {
            return false;
        }
    } else {
        // it's a normal identifier
        if (!lexer_add_token_identifier(l, sp, p)) {
            return false;
        }
    }

    if (p != lim) {
        *ret_p = p + 1;
    } else {
        *ret_p = p;
        l->at_eof = true;
    }

    return true;
}

#define COND_NUMERIC(p_)                        \
    ((p_) >= '0' && (p_) <= '9')
static bool lexer_get_numeric(struct lexer *l, char *p,
                              char *lim, char **ret_p)
{
    char *sp = p;
    while (p < lim) {
        if (COND_NUMERIC(*(p+1))) {
            p ++;
        } else {
            break;
        }
    }

    if (!lexer_add_token_numeric(l, sp, p)) {
        return false;
    }

    if (p != lim) {
        *ret_p = p + 1;
    } else {
        *ret_p = p;
    }

    return true;
}

#define COND_TOKEN_AMBIG1(p_)                     \
    ((p_) == '+' || (p_) == '-' || (p_) == '>' || \
     (p_) == '<' || (p_) == '|' || (p_) == '=')

bool lexer_scan(struct lexer *l)
{
    char *lim;
    char *sp;
    char *p;

    sp = p = inpfile_sp(l->file);
    lim = sp + rf_string_length_bytes(inpfile_str(l->file)) - 1;

   /* TODO: combine inpfile_at_eof with lexer eof check */
    while (!l->at_eof && p <= lim) {
        inpfile_acc_ws(l->file);
        if (inpfile_at_eof(l->file)) {
            break;
        }
        p = inpfile_p(l->file);
        sp = p;

        if (COND_IDENTIFIER_BEGIN(*p)) {
            if (!lexer_get_identifier(l, p, lim, &p)) {
                return false;
            }
        } else if (COND_NUMERIC(*p)) {
            if (!lexer_get_numeric(l, p, lim, &p)) {
                return false;
            }
        } else { // see if it's a token
            unsigned int len = 1;
            const struct internal_token *itoken;
            const struct internal_token *itoken2;
            bool got_token = false;
            char * toksp = p;

            while (len <= MAX_WORD_LENGTH) {
                itoken = lexer_lexeme_is_token(p, len);
                if (itoken) {
                    /* if more than 1 tokens may start with that character */
                    if (p + 1 <= lim && COND_TOKEN_AMBIG1(*toksp)) {
                        len = 2;
                        itoken2 = lexer_lexeme_is_token(p, len);
                        if (itoken2) {
                            itoken = itoken2;
                        } else {
                            len = 1;
                        }
                    }
                    if (!lexer_add_token(l, itoken->type, toksp, p + len - 1)) {
                        return false;
                    }
                    p += len;
                    got_token=true;
                    break;
                }
                len ++;
            }
            if (!got_token) {
                // error unknown token
                lexer_synerr(l, &lexer_get_top_token(l)->location.start, NULL,
                               "Unknown token encountered");
                return false;
            }
        }
        inpfile_move(l->file, p - sp, p - sp);
    }
    return true;
}

// convenience macro to check if a token index is out of bounds for the lexer
#define LEXER_IND_OOB(lex_, ind_)               \
    (ind_ >= darray_size((lex_)->tokens))

struct token *lexer_next_token(struct lexer *l)
{
    struct token *tok;
    if (LEXER_IND_OOB(l, l->tok_index)) {
        return NULL;
    }
    tok = &darray_item(l->tokens, l->tok_index);
    l->tok_index ++;
    return tok;
}

struct token *lexer_lookahead(struct lexer *l, unsigned int num)
{
    struct token *tok;
    unsigned int index = l->tok_index + num - 1;
    if (LEXER_IND_OOB(l, index)) {
        return NULL;
    }
    tok = &darray_item(l->tokens, index);
    return tok;
}

struct token *lexer_last_token_valid(struct lexer *l)
{
    if (LEXER_IND_OOB(l, l->tok_index)) {
        return &darray_item(l->tokens, l->tok_index - 1);
    }
    return &darray_item(l->tokens, l->tok_index);
}

i_INLINE_INS struct inplocation *lexer_last_token_location(struct lexer *l);
i_INLINE_INS struct inplocation_mark *lexer_last_token_start(struct lexer *l);
i_INLINE_INS struct inplocation_mark *lexer_last_token_end(struct lexer *l);

void lexer_push(struct lexer *l)
{
    darray_append(l->indices, l->tok_index);
}

void lexer_pop(struct lexer *l)
{
    RF_ASSERT(!darray_empty(l->indices));
    (void)darray_pop(l->indices);
}

void lexer_rollback(struct lexer *l)
{
    unsigned int idx;
    RF_ASSERT(!darray_empty(l->indices));
    idx = darray_pop(l->indices);
    l->tok_index = idx;
}
