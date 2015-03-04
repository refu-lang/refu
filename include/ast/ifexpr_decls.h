#ifndef LFR_AST_IF_EXPRESSION_DECLS_H
#define LFR_AST_IF_EXPRESSION_DECLS_H

struct ast_node;

struct ast_condbranch {
    //! An expression containing the condition for this branch
    struct ast_node *cond;
    //! A block containing the body of the condition
    struct ast_node *body;
};

struct ast_ifexpr {
    //! A conditional branch for the 'if' of the if expression
    struct ast_node *taken_branch;
    //! The block for the else code. Can be one of:
    //!  + another ifexpr signifying an elif
    //!  + a simple block signifying an else
    //!  + NULL signifying absence of elif/else
    struct ast_node *fallthrough_branch;
};

#endif
