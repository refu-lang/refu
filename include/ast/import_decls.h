#ifndef LFR_AST_IMPORT_DECLS_H
#define LFR_AST_IMPORT_DECLS_H

#include <stdbool.h>
#include <Data_Structures/darray.h>

struct ast_node;

struct ast_import {
    //! Distinguish between foreign and normal imports
    bool foreign;
    //! Array of the importees
    struct {darray(struct ast_node*);} member;
};
#endif
