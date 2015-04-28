#ifndef LFR_AST_MATCHEXPR_DECLS_H
#define LFR_AST_MATCHEXPR_DECLS_H

#include <stdlib.h>

struct ast_matchcase {
    struct ast_node *pattern;
    struct ast_node *expression;
    //! A pointer to the type that the pattern of this case matched
    const struct type *matched_type;
    //! Symbol table of this matchcase
    //! Points to the symbol table of the matchcase pattern
    struct symbol_table *st;
};

struct ast_matchexpr {
    struct ast_node *identifier;
    size_t match_cases_num;
};

#endif
