#ifndef LFR_AST_GENERICS_DECLS_H
#define LFR_AST_GENERICS_DECLS_H

struct ast_node;

struct ast_genrtype {
    struct ast_node *type;
    struct ast_node *id;
};

#endif
