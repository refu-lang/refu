#ifndef LFR_AST_MATCHEXPR_DECLS_H
#define LFR_AST_MATCHEXPR_DECLS_H

struct ast_matchcase {
    struct ast_node *pattern;
    struct ast_node *expression;
};

struct ast_matchexpr {
    struct ast_node *identifier;
};

#endif
