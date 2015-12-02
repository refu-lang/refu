#ifndef LFR_LEXER_TOKENS_H
#define LFR_LEXER_TOKENS_H

#include <Definitions/inline.h>
#include <Utils/sanity.h>

#include <inplocation.h>
#include <ast/ast.h>

struct RFstring;
struct token;

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
enum token_type {
    TOKEN_IDENTIFIER = 0,
    TOKEN_CONSTANT_INTEGER,
    TOKEN_CONSTANT_FLOAT,
    TOKEN_STRING_LITERAL,

    /* symbols common between all lexers */
    TOKEN_SM_OCBRACE,
    TOKEN_SM_CCBRACE,
    TOKEN_SM_OSBRACE,
    TOKEN_SM_CSBRACE,
    TOKEN_SM_OPAREN,
    TOKEN_SM_CPAREN,
    TOKEN_SM_DBLQUOTE,
    TOKEN_SM_COLON,
    TOKEN_SM_THICKARROW,

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
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,
    TOKEN_KW_MATCH,
    TOKEN_KW_MODULE,
    TOKEN_KW_IMPORT,
    TOKEN_KW_FOREIGN_IMPORT,

    TOKENS_MAX
};


/*
 * A token's value. Only for tokens that form a full ast_node.
 * Also contains memory ownership semantics
 */
struct tok_value {
    union {
        struct ast_node *ast;
        // TODO: Add more types of valus here
    } value;
    bool owned_by_lexer;
};

struct token {
    enum token_type type;
    struct inplocation location;
    struct tok_value value;
};

const struct RFstring *tokentype_to_str(enum token_type type);

i_INLINE_DECL const struct inplocation *token_get_loc(const struct token *tok)
{
    return &tok->location;
}

i_INLINE_DECL const struct inplocation_mark *token_get_start(const struct token *tok)
{
    return &tok->location.start;
}

i_INLINE_DECL const struct inplocation_mark *token_get_end(const struct token *tok)
{
    return &tok->location.end;
}

i_INLINE_DECL bool token_is_binaryop(const struct token *tok)
{
    return ((tok->type >= TOKEN_OP_PLUS && tok->type <= TOKEN_OP_BITWISE_XOR) ||
            tok->type == TOKEN_OP_AMPERSAND ||
            tok->type == TOKEN_OP_COMMA     ||
            tok->type == TOKEN_OP_TYPESUM   ||
            tok->type == TOKEN_SM_OSBRACE);
}

i_INLINE_DECL bool token_is_unaryop(const struct token *tok)
{

    return ((tok->type >= TOKEN_OP_AMPERSAND && tok->type <= TOKEN_OP_DEC) ||
            tok->type == TOKEN_OP_MINUS || tok->type == TOKEN_OP_PLUS);
}

i_INLINE_DECL bool token_is_prefix_unaryop(const struct token *tok)
{

    return token_is_unaryop(tok);
}

i_INLINE_DECL bool token_is_postfix_unaryop(const struct token *tok)
{

    return tok->type == TOKEN_OP_INC || tok->type == TOKEN_OP_DEC;
}

i_INLINE_DECL bool token_is_numeric_constant(const struct token *tok)
{
    return tok->type == TOKEN_CONSTANT_INTEGER || tok->type == TOKEN_CONSTANT_FLOAT;
}

i_INLINE_DECL bool token_has_value(const struct token *tok)
{
    return tok->type == TOKEN_IDENTIFIER ||
        tok->type == TOKEN_STRING_LITERAL ||
        tok->type == TOKEN_CONSTANT_INTEGER ||
        tok->type == TOKEN_CONSTANT_FLOAT;
}

// gets the value from a token. Use only after a tok->type check
i_INLINE_DECL struct ast_node *token_get_value(struct token *tok)
{
    RF_ASSERT(token_has_value(tok), "Requesting value of illegal token type");
    tok->value.owned_by_lexer = false;
    return tok->value.value.ast;
}


#endif
