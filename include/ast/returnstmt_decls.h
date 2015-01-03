#ifndef LFR_AST_RETURNSTMT_DECLS_H
#define LFR_AST_RETURNSTMT_DECLS_H

struct ast_node;

struct ast_returnstmt {
    //! The expression to return
    struct ast_node *expr;
};
#endif
