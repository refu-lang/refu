#ifndef LFR_AST_ARGUMENT_H
#define LFR_AST_ARGUMENT_H

#include <rfbase/datastructs/darray.h>

struct ast_argument {
    struct RFstring *name;
    struct type *type;
};

//! An array of arguments
struct arr_arguments {darray(struct ast_argument*);};

struct ast_argument *ast_argument_create(struct RFstring *name, struct type *t);
void ast_argument_destroy(struct ast_argument *a);



#endif
