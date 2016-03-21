#ifndef LFR_AST_ARR_DECLS_H
#define LFR_AST_ARR_DECLS_H

#include <rflib/datastructs/darray.h>
#include <ast/ast_utils.h>
struct ast_node;

//! An array specifier. The part of a type that specifies an array.
struct ast_arrspec {
    struct arr_ast_nodes dimensions;
};

#endif
