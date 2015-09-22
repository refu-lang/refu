#ifndef LFR_AST_FUNCTIONS_DECLS_H
#define LFR_AST_FUNCTIONS_DECLS_H

#include <analyzer/symbol_table.h>

struct ast_node;
struct inplocation_mark;
struct type;

//! The position in the code where a function declaration is found
enum fndecl_position {
    FNDECL_STANDALONE,
    FNDECL_PARTOF_IMPL,
    FNDECL_PARTOF_TYPECLASS,
    FNDECL_PARTOF_FOREIGN_IMPORT
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
    //! Number of function arguments TODO : remove when rir_function is used instead
    unsigned args_num;

    //! Symbol table of the function's arguments and return values.
    //! Only initialized in analyzer phase.
    //! Not using top level type descriptions with their own symbol tables since
    //! having it here makes it easier to combine symbol from both args and return
    struct symbol_table st;
};

struct ast_fnimpl {
    //! The function's declaration (signature)
    struct ast_node *decl;
    //! The function's body, either a block or a match expression
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
    //! Type that matched this particular function call's parameters
    //! during typechecking.
    const struct type *params_type;
    //! Optional: generic attribute
    struct ast_node *genr;
};
#endif
