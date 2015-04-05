#ifndef LFR_AST_TYPE_DECLS_H
#define LFR_AST_TYPE_DECLS_H

#include <analyzer/symbol_table.h>
#include <types/type_decls.h>

struct ast_node;

struct ast_typeop {
    //! Type Operator type
    enum typeop_type type;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_typedesc {
    struct ast_node *left;
    struct ast_node *right;
    //! Symbol table of the identifiers inside the type declaration
    struct symbol_table st;
};

struct ast_typeleaf {
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_typedecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
    //! Symbol table of the identifiers inside the type declaration
    struct symbol_table st;

    //! Optional, generic declaration
    //! TODO: This is not yet implemented!
    struct ast_node *genrdecl;
};
#endif
