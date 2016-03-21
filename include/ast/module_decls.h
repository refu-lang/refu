#ifndef LFR_AST_MODULE_DECLS_H
#define LFR_AST_MODULE_DECLS_H

#include <stdbool.h>
#include <rflib/datastructs/darray.h>
#include <ast/ast_utils.h>

struct ast_node;

struct ast_import {
    //! Distinguish between foreign and normal imports
    bool foreign;
    //! Array of the importees
    struct arr_ast_nodes member;
};

struct ast_module {
    //! Identifier with the name of the module
    struct ast_node *name;
    //! If the module has arguments, this contains their type description. If not NULL.
    struct ast_node *args;
    //! Symbol table of the module's arguments and of its contents
    struct symbol_table st;
};
#endif
