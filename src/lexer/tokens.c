#include <lexer/tokens.h>

#include <String/rf_str_core.h>
#include <Utils/build_assert.h>

static struct RFstring strings_[] = {
    RF_STRING_STATIC_INIT("identifier"),
    RF_STRING_STATIC_INIT("constant integer"),
    RF_STRING_STATIC_INIT("constant float"),
    RF_STRING_STATIC_INIT("string literal"),

    /* keywords */
    RF_STRING_STATIC_INIT("const"),
    RF_STRING_STATIC_INIT("type"),
    RF_STRING_STATIC_INIT("fn"),
    RF_STRING_STATIC_INIT("class"),
    RF_STRING_STATIC_INIT("if"),
    RF_STRING_STATIC_INIT("elif"),
    RF_STRING_STATIC_INIT("else"),

    /* symbols */
    RF_STRING_STATIC_INIT(":"),
    RF_STRING_STATIC_INIT("{"),
    RF_STRING_STATIC_INIT("}"),
    RF_STRING_STATIC_INIT("("),
    RF_STRING_STATIC_INIT(")"),
    RF_STRING_STATIC_INIT("\""),

    /* binary operators */
    RF_STRING_STATIC_INIT("+"),
    RF_STRING_STATIC_INIT("-"),
    RF_STRING_STATIC_INIT("*"),
    RF_STRING_STATIC_INIT("/"),

    /* binary comparsison operators */
    RF_STRING_STATIC_INIT("=="),
    RF_STRING_STATIC_INIT("!="),
    RF_STRING_STATIC_INIT(">"),
    RF_STRING_STATIC_INIT(">="),
    RF_STRING_STATIC_INIT("<"),
    RF_STRING_STATIC_INIT("<="),

    /* unary operators*/
    RF_STRING_STATIC_INIT("&"),
    RF_STRING_STATIC_INIT("++"),
    RF_STRING_STATIC_INIT("--"),
    RF_STRING_STATIC_INIT("="),

    /* type operators */
    RF_STRING_STATIC_INIT("|"),
    RF_STRING_STATIC_INIT(","),
    RF_STRING_STATIC_INIT("->"),

    /* boolean operators */
    RF_STRING_STATIC_INIT("&&"),
    RF_STRING_STATIC_INIT("||"),
};

const struct RFstring *tokentype_to_str(enum token_type type)
{
    BUILD_ASSERT(sizeof(strings_) / sizeof(struct RFstring) == TOKENS_MAX);
    
    return &strings_[type];
}
