#ifndef LFR_AST_VARDECL_DECLS_H
#define LFR_AST_VARDECL_DECLS_H

struct ast_node;

struct ast_vardecl {
    //! identifier of the name
    struct ast_node *name;
    //! identifier of the type
    struct ast_node *type;
};

#endif
