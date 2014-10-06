#ifndef LFR_AST_VARDECL_DECLS_H
#define LFR_AST_VARDECL_DECLS_H

struct ast_node;

struct ast_vardecl {
    //! Type description of the variable declaration
    struct ast_node *desc;
};

#endif
