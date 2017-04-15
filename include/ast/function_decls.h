#ifndef LFR_AST_FUNCTIONS_DECLS_H
#define LFR_AST_FUNCTIONS_DECLS_H

#include <analyzer/symbol_table.h>
#include <ast/ast_utils.h>
#include <ast/argument.h>

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
    struct arr_arguments arguments;

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

enum ast_fncall_type {
    AST_FNCALL_NORMAL = 0,
    //! Set by typecheking if this is an explicit conversion
    AST_FNCALL_EXPLICIT_CONVERSION,
    //! Set by typechecking if this is a call to a function with sum type args
    AST_FNCALL_SUM,
    //! Set by typecheking if this is a foreign function call
    AST_FNCALL_FOREIGN,
};

struct ast_fncall {
    //! identifier of the name
    struct ast_node *name;
    //! Expression describing the function arguments or NULL if there is no arguments
    struct ast_node *args;
    //! Type that matched this particular function call's parameters
    //! during typechecking.
    const struct type *params_type;
    //! Type of the original function this call refers to. Set during typechecking
    const struct type *declared_type;
    //! Type of call
    enum ast_fncall_type type;
    //! An array of all the arguments of this function call
    //! Populated at the beginning of typechecking.
    struct arr_ast_nodes arguments;
    //! Optional: generic attribute
    struct ast_node *genr;
};
#endif
