#ifndef LFR_AST_TYPE_DECLS_H
#define LFR_AST_TYPE_DECLS_H

#include <analyzer/symbol_table.h>

struct ast_node;

enum typeop_type {
    TYPEOP_INVALID,
    TYPEOP_SUM,
    TYPEOP_PRODUCT,
    TYPEOP_IMPLICATION
};

struct ast_typeop {
    //! Type Operator type
    enum typeop_type type;
    struct ast_node *left;
    struct ast_node *right;
};
#define ast_typeop_to_node(n_)                    \
    container_of((n_), struct ast_node, typeop)

struct ast_typedesc {
    struct ast_node *left;
    struct ast_node *right;
};
#define ast_typedesc_to_node(n_)                  \
    container_of((n_), struct ast_node, typedesc)

struct ast_typedecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
    //! Symbol table for the type description
    struct symbol_table st;
};
#endif
