#ifndef LFR_AST_ARRDECLS_H
#define LFR_AST_ARRDECLS_H

#include <ast/ast_utils.h>

struct ast_bracketlist {
    //! An array of all the members of this bracketlist
    //! Populated at the beginning of typechecking.
    struct arr_ast_nodes members;
};

#endif
