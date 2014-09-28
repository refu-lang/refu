#ifndef LFR_LEXER_TOKENS_H
#define LFR_LEXER_TOKENS_H

struct RFstring;
enum token_type {
    TOKEN_IDENTIFIER = 0,
    TOKEN_CONSTANT_INTEGER,
    TOKEN_CONSTANT_FLOAT,
    TOKEN_STRING_LITERAL,


    /* keywords */
    TOKEN_KW_CONST,
    TOKEN_KW_TYPE,
    TOKEN_KW_FUNCTION,
    TOKEN_KW_TYPECLASS,
    TOKEN_KW_IF,
    TOKEN_KW_ELIF,
    TOKEN_KW_ELSE,

    /* symbols */
    TOKEN_SM_COLON,
    TOKEN_SM_OCBRACE,
    TOKEN_SM_CCBRACE,
    TOKEN_SM_OPAREN,
    TOKEN_SM_CPAREN,
    TOKEN_SM_DBLQUOTE,

    /* binary operators */
    TOKEN_OP_PLUS,
    TOKEN_OP_MINUS,
    TOKEN_OP_MULTI,
    TOKEN_OP_DIV,

    /* binary comparsison operators */
    TOKEN_OP_EQ,
    TOKEN_OP_NEQ,
    TOKEN_OP_GT,
    TOKEN_OP_GTEQ,
    TOKEN_OP_LT,
    TOKEN_OP_LTEQ,

    /* unary operators*/
    TOKEN_OP_AMPERSAND,
    TOKEN_OP_INC,
    TOKEN_OP_DEC,
    TOKEN_OP_ASSIGN,

    /* type operators */
    TOKEN_OP_TYPESUM,
    TOKEN_OP_COMMA,
    TOKEN_OP_IMPL,

    /* boolean operators */
    TOKEN_OP_LOGICAND,
    TOKEN_OP_LOGICOR,


    TOKENS_MAX
};

const struct RFstring *tokentype_to_str(enum token_type type);

#endif
