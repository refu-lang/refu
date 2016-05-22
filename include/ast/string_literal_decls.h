#ifndef LFR_AST_STRING_LITERAL_DECLS_H
#define LFR_AST_STRING_LITERAL_DECLS_H

#include <rfbase/string/decl.h>

struct ast_string_literal {
    struct RFstring string;
    uint32_t hash;
};
#endif
