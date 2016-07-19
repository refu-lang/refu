#ifndef LFR_AST_FOR_EXPRESSION_DECLS_H
#define LFR_AST_FOR_EXPRESSION_DECLS_H

struct ast_node;

struct ast_forexpr {
    //! The loop variable that changes in each iteration
    struct ast_node *loopvar;
    //! The iterable object
    struct ast_node *iterable;
    //! The body of the loop, code to execute in each iteration
    struct ast_node *body;
};

#endif
