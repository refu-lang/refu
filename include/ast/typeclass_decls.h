#ifndef LFR_AST_TYPECLASS_DECLS_H
#define LFR_AST_TYPECLASS_DECLS_H

#include <analyzer/symbol_table.h>

struct ast_typeclass {
    struct ast_node *name;
    struct ast_node *generics;
    //! The symbol table for the typeclass
    struct symbol_table st;
};

struct ast_typeinstance {
    bool is_default;
    struct ast_node *class_name;
    struct ast_node *type_name;
    struct ast_node *instance_name;

    //! The symbol table for the typeinstance
    struct symbol_table st;
};

#endif
