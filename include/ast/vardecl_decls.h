#ifndef LFR_AST_VARDECL_DECLS_H
#define LFR_AST_VARDECL_DECLS_H

struct ast_node;

struct ast_vardecl {
    //! Type leaf of the type description
    struct ast_node *leaf;
};

#endif
