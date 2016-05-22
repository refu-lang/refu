#include <lexer/tokens.h>

#include <rfbase/string/core.h>
#include <rfbase/utils/build_assert.h>

static struct RFstring strings_[] = {
    RF_STRING_STATIC_INIT("identifier"),
    RF_STRING_STATIC_INIT("constant integer"),
    RF_STRING_STATIC_INIT("constant float"),
    RF_STRING_STATIC_INIT("string literal"),

    /* symbols */
    RF_STRING_STATIC_INIT("{"),
    RF_STRING_STATIC_INIT("}"),
    RF_STRING_STATIC_INIT("["),
    RF_STRING_STATIC_INIT("]"),
    RF_STRING_STATIC_INIT("("),
    RF_STRING_STATIC_INIT(")"),
    RF_STRING_STATIC_INIT("\""),
    RF_STRING_STATIC_INIT(":"),
    RF_STRING_STATIC_INIT("=>"),

    /* binary operators */
    RF_STRING_STATIC_INIT("+"),
    RF_STRING_STATIC_INIT("-"),
    RF_STRING_STATIC_INIT("*"),
    RF_STRING_STATIC_INIT("/"),
    RF_STRING_STATIC_INIT("="),

    /* binary comparison operators */
    RF_STRING_STATIC_INIT("=="),
    RF_STRING_STATIC_INIT("!="),
    RF_STRING_STATIC_INIT(">"),
    RF_STRING_STATIC_INIT(">="),
    RF_STRING_STATIC_INIT("<"),
    RF_STRING_STATIC_INIT("<="),

    /* logic binary operators */
    RF_STRING_STATIC_INIT("&&"),
    RF_STRING_STATIC_INIT("||"),

    /* other binary operators */
    RF_STRING_STATIC_INIT("."),

    /* bitwise binary operators */
    RF_STRING_STATIC_INIT("|"),
    RF_STRING_STATIC_INIT("&"),
    RF_STRING_STATIC_INIT("^"),

    /* unary operators*/
    RF_STRING_STATIC_INIT("&"),
    RF_STRING_STATIC_INIT("++"),
    RF_STRING_STATIC_INIT("--"),

    /* type operators */
    RF_STRING_STATIC_INIT("|"),
    RF_STRING_STATIC_INIT(","),
    RF_STRING_STATIC_INIT("->"),

    /* keywords */
    RF_STRING_STATIC_INIT("const"),
    RF_STRING_STATIC_INIT("type"),
    RF_STRING_STATIC_INIT("fn"),
    RF_STRING_STATIC_INIT("class"),
    RF_STRING_STATIC_INIT("instance"),
    RF_STRING_STATIC_INIT("if"),
    RF_STRING_STATIC_INIT("elif"),
    RF_STRING_STATIC_INIT("else"),
    RF_STRING_STATIC_INIT("return"),
    RF_STRING_STATIC_INIT("true"),
    RF_STRING_STATIC_INIT("false"),
    RF_STRING_STATIC_INIT("match"),
    RF_STRING_STATIC_INIT("module"),
    RF_STRING_STATIC_INIT("import"),
    RF_STRING_STATIC_INIT("foreign_import"),
};

const struct RFstring *tokentype_to_str(enum token_type type)
{
    // helps make sure that the strings array is in sync with the tokens
    BUILD_ASSERT(sizeof(strings_) / sizeof(struct RFstring) == TOKENS_MAX);

    return &strings_[type];
}

i_INLINE_INS const struct inplocation *token_get_loc(const struct token *tok);
i_INLINE_INS const struct inplocation_mark *token_get_start(const struct token *tok);
i_INLINE_INS const struct inplocation_mark *token_get_end(const struct token *tok);

i_INLINE_INS bool token_is_binaryop(const struct token *tok);
i_INLINE_INS bool token_is_unaryop(const struct token *tok);
i_INLINE_INS bool token_is_prefix_unaryop(const struct token *tok);
i_INLINE_INS bool token_is_postfix_unaryop(const struct token *tok);
i_INLINE_INS bool token_is_numeric_constant(const struct token *tok);
