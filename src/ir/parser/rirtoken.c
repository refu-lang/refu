#include <ir/parser/rirtoken.h>

#include <rflib/string/core.h>
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
    RF_STRING_STATIC_INIT("fndecl"),
    RF_STRING_STATIC_INIT("identifier variable"),
    RF_STRING_STATIC_INIT("identifier label"),
    RF_STRING_STATIC_INIT("return"),
    RF_STRING_STATIC_INIT("branch"),
    RF_STRING_STATIC_INIT("condbranch"),
    RF_STRING_STATIC_INIT("convert"),
    RF_STRING_STATIC_INIT("write"),
    RF_STRING_STATIC_INIT("read"),
    RF_STRING_STATIC_INIT("call"),

    RF_STRING_STATIC_INIT("add"),
    RF_STRING_STATIC_INIT("sub"),
    RF_STRING_STATIC_INIT("mul"),
    RF_STRING_STATIC_INIT("div"),
    RF_STRING_STATIC_INIT("cmpeq"),
    RF_STRING_STATIC_INIT("cmpne"),
    RF_STRING_STATIC_INIT("cmpgt"),
    RF_STRING_STATIC_INIT("cmpge"),
    RF_STRING_STATIC_INIT("cmplt"),
    RF_STRING_STATIC_INIT("cmple")
};

const struct RFstring *rir_tokentype_to_str(enum rir_token_type type)
{
    // helps make sure that the strings array is in sync with the tokens
    BUILD_ASSERT(sizeof(strings_) / sizeof(struct RFstring) == RIR_TOKENS_MAX);

    return &strings_[type];
}

