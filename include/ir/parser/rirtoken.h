#ifndef LFR_RIR_TOKEN_H
#define LFR_RIR_TOKEN_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <inplocation.h>
#include <ast/ast.h>

struct RFstring;
struct rinternal_token;

/**
 * The tokens of the language
 *
 * Note that the groups they are grouped in are not necessarily
 * correct. For example some tokens can be both binary and unary operators.
 * For correct checking of whethere a token belongs to a certain group we have
 * the family of TOKEN_IS_XX macros. For example:
 *
 * TOKEN_IS_BINARY_OP()
 * TOKEN_IS_UNARY_OP()
 * TOKEN_IS_NUMERIC_CONSTANT()
 */
enum rir_token_type {
    RIR_TOK_IDENTIFIER = 0,
    RIR_TOK_CONTANT_INTEGER,
    RIR_TOK_CONSTANT_FLOAT,
    RIR_TOK_STRING_LITERAL,

    RIR_TOK_SM_OCBRACE,
    RIR_TOK_SM_CCBRACE,
    RIR_TOK_SM_OSBRACE,
    RIR_TOK_SM_CSBRACE,
    RIR_TOK_SM_OPAREN,
    RIR_TOK_SM_CPAREN,
    RIR_TOK_SM_DBLQUOTE,
    RIR_TOK_SM_COMMA,

    RIR_TOK_OP_MINUS,
    RIR_TOK_OP_MULTI,
    RIR_TOK_OP_ASSIGN,

    RIR_TOK_SEMICOLON,

    RIR_TOK_GLOBAL,
    RIR_TOK_UNIONDEF,
    RIR_TOK_TYPEDEF,
    RIR_TOK_FNDEF,
    RIR_TOK_FNDECL,
    RIR_TOK_IDENTIFIER_VARIABLE,
    RIR_TOK_IDENTIFIER_LABEL,
    RIR_TOK_RETURN,
    RIR_TOK_BRANCH,
    RIR_TOK_CONDBRANCH,
    RIR_TOK_CONVERT,

    RIR_TOKENS_MAX
};

/**
 * Convenience macro to convert from a normal to rir token type and
 * help us avoid writting out the cast each time when using a token
 */
#define rir_toktype(i_tok_) ((enum rir_token_type)(i_tok_)->type)


const struct rinternal_token *rir_lexer_lexeme_is_token (register const char *str,
                                                         register unsigned int len);

const struct RFstring *rir_tokentype_to_str(enum rir_token_type type);

#endif
