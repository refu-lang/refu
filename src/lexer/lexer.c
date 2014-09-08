#include <lexer/lexer.h>
#include "tokens_htable.h" /* include the gperf generated hash table */

#include <parser/file.h>


static inline bool token_init(struct token *t,
                              enum token_type type,
                              struct parser_file *f,
                              char *sp, char *ep)
{
    t->type = type;
    if (!ast_location_init(&t->loc, f, sp, ep)) {
        return false;
    }
    return true;
}

static inline bool token_init_identifier(struct token *t,
                                         struct parser_file *f,
                                         char *sp, char *ep)
{
    if (!token_init(t, TOKEN_IDENTIFIER, f, sp, ep)) {
        return false;
    }
    RF_STRING_SHALLOW_INIT(&t->value.string, sp, ep - sp + 1);
    return true;
}

static inline bool token_init_numeric(struct token *t,
                                      struct parser_file *f,
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

bool lexer_init(struct lexer *l)
{
    darray_init(l->tokens);
    darray_init(l->indices);
    return true;
}
void lexer_deinit(struct lexer *l)
{
    darray_free(l->tokens);
    darray_free(l->indices);
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

bool lexer_scan(struct lexer *l, struct parser_file *f)
{

    char *lim;
    char *sp;
    char *p;
    l->file = f;

    sp = p = parser_file_sp(f);
    lim = sp + rf_string_length_bytes(parser_file_str(f)) - 1;
    unsigned int i = 0;
    int z = 0;
    while (p < lim) {
        if (i == 14) {
            z = z + 1;
        }
        parser_file_acc_ws(f);
        p = parser_file_p(f);
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
                    if (COND_TOKEN_AMBIG1(*toksp)) {
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
                parser_file_synerr(f, -len,
                                   "Unknown token encountered");
                printf("err 4");
                return false;
            }
        }
        parser_file_move(f, p - sp, p - sp);
        i++;
    }
    return true;
}
