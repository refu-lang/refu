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
    TOKEN_KW_TYPEINSTANCE,
    TOKEN_KW_IF,
    TOKEN_KW_ELIF,
    TOKEN_KW_ELSE,
    TOKEN_KW_RETURN,

    /* symbols */
    TOKEN_SM_COLON,
    TOKEN_SM_OCBRACE,
    TOKEN_SM_CCBRACE,
    TOKEN_SM_OSBRACE,
    TOKEN_SM_CSBRACE,
    TOKEN_SM_OPAREN,
    TOKEN_SM_CPAREN,
    TOKEN_SM_DBLQUOTE,

    /* binary operators (keep grouped with all binary operators) */
    TOKEN_OP_PLUS,
    TOKEN_OP_MINUS,
    TOKEN_OP_MULTI,
    TOKEN_OP_DIV,
    TOKEN_OP_ASSIGN,

    /* binary comparison operators (keep grouped with all binary operators) */
    TOKEN_OP_EQ,
    TOKEN_OP_NEQ,
    TOKEN_OP_GT,
    TOKEN_OP_GTEQ,
    TOKEN_OP_LT,
    TOKEN_OP_LTEQ,

    /* logic binary operators (keep grouped with all binary operators) */
    TOKEN_OP_LOGIC_AND,
    TOKEN_OP_LOGIC_OR,

    /* other binary operators (keep grouped with all binary operators) */
    TOKEN_OP_MEMBER_ACCESS,

    /* bitwise binary operators (keep grouped with all binary operators) */
    TOKEN_OP_BITWISE_OR, // at least for now same as type sum operator, so this is ignored in the lexing stage
    TOKEN_OP_BITWISE_AND, // at least for now same as TOKEN_OP_AMPERSAND, so this is ignored in the lexing stage
    TOKEN_OP_BITWISE_XOR,

    /* unary operators*/
    TOKEN_OP_AMPERSAND,
    TOKEN_OP_INC,
    TOKEN_OP_DEC,

    /* type operators */
    TOKEN_OP_TYPESUM,
    TOKEN_OP_COMMA,
    TOKEN_OP_IMPL,

    TOKENS_MAX
};


#define TOKEN_IS_BINARY_OP(tok_)                \
    (((tok_)->type >= TOKEN_OP_PLUS &&          \
      (tok_)->type <= TOKEN_OP_BITWISE_XOR) ||  \
     (tok_)->type == TOKEN_OP_AMPERSAND ||      \
     (tok_)->type == TOKEN_OP_COMMA     ||      \
     (tok_)->type == TOKEN_OP_TYPESUM   ||      \
     (tok_)->type == TOKEN_SM_OSBRACE)

#define TOKEN_IS_UNARY_OP(tok_)                \
    ((tok_)->type >= TOKEN_OP_AMPERSAND &&     \
     (tok_)->type <= TOKEN_OP_DEC)

#define TOKEN_IS_NUMERIC_CONSTANT(tok_)         \
    ((tok_)->type == TOKEN_CONSTANT_INTEGER ||  \
     (tok_)->type == TOKEN_CONSTANT_FLOAT)

const struct RFstring *tokentype_to_str(enum token_type type);

#endif
