#ifndef LFR_AST_MATCHEXPR_DECLS_H
#define LFR_AST_MATCHEXPR_DECLS_H

#include <stdlib.h>

struct ast_matchcase {
    //! A type description of the match pattern
    struct ast_node *pattern;
    //! An expression to follow when pattern matches
    struct ast_node *expression;
    //! A pointer to the type that the pattern of this case matched
    const struct type *matched_type;
    //! The index of matched type in the original type
    int match_idx;
    //! Symbol table of this matchcase
    //! Points to the symbol table of the matchcase pattern
    struct symbol_table *st;
};

struct ast_matchexpr {
    //! A pointer to either the identifier of the match node
    //! or to the arguments of the parent function if this is
    //! a headless match expression
    struct ast_node *identifier_or_fnargtype;
    //! The type that the match expression tries to match
    const struct type *matching_type;
    //! Number of match cases under the expression
    size_t match_cases_num;
};

#endif
