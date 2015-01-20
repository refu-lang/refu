#ifndef LFR_AST_FUNCTIONS_DECLS_H
#define LFR_AST_FUNCTIONS_DECLS_H

#include <analyzer/symbol_table.h>

struct ast_node;
struct inplocation_mark;

//! The position in the code where a function declaration is found
enum fndecl_position {
    FNDECL_STANDALONE,
    FNDECL_PARTOF_IMPL,
    FNDECL_PARTOF_TYPECLASS,
};

struct ast_fndecl {
    //! identifier of the name
    struct ast_node *name;
    //! Optional: generic declaration
    struct ast_node *genr;
    //! type description of the arguments
    struct ast_node *args;
    //! Optional: type description of the return value
    struct ast_node *ret;

    //! Position of function declaration. Basically where it's found in the code
    enum fndecl_position position;
    //! Symbol table of the function's arguments. Only initialized in analyzer phase
    struct symbol_table st;
};

struct ast_fnimpl {
    //! The function's declaration (signature)
    struct ast_node *decl;
    //! The function's body
    struct ast_node *body;
    //! Symbol table of the function's arguments.
    //! Points to the symbol table of the declaration
    struct symbol_table *st;
};

struct ast_fncall {
    //! identifier of the name
    struct ast_node *name;
    //! Expression describing the function arguments or NULL if there is no arguments
    struct ast_node *args;
    //! Optional: generic attribute
    struct ast_node *genr;
};
#endif
