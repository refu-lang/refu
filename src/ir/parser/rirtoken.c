#include <ir/parser/rirtoken.h>
#include "rirtoken_htable.h"

static struct RFstring strings_[] = {
    RF_STRING_STATIC_INIT("identifier"),
    RF_STRING_STATIC_INIT("constant integer"),
    RF_STRING_STATIC_INIT("constant float"),
    RF_STRING_STATIC_INIT("string literal"),

    RF_STRING_STATIC_INIT("{"),
    RF_STRING_STATIC_INIT("}"),
    RF_STRING_STATIC_INIT("["),
    RF_STRING_STATIC_INIT("]"),
    RF_STRING_STATIC_INIT("("),
    RF_STRING_STATIC_INIT(")"),
    RF_STRING_STATIC_INIT("\""),
    RF_STRING_STATIC_INIT(","),

    /* binary operators */
    RF_STRING_STATIC_INIT("-"),
    RF_STRING_STATIC_INIT("*"),
    RF_STRING_STATIC_INIT("="),

    RF_STRING_STATIC_INIT(";"),

    /* keywords */
    RF_STRING_STATIC_INIT("global"),
    RF_STRING_STATIC_INIT("uniondef"),
    RF_STRING_STATIC_INIT("typedef"),
    RF_STRING_STATIC_INIT("fndef"),
    RF_STRING_STATIC_INIT("identifier variable"),
    RF_STRING_STATIC_INIT("identifier label"),
};

const struct RFstring *rir_tokentype_to_str(enum rir_token_type type)
{
    // helps make sure that the strings array is in sync with the tokens
    BUILD_ASSERT(sizeof(strings_) / sizeof(struct RFstring) == RIR_TOKENS_MAX);

    return &strings_[type];
}

