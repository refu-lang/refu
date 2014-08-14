#ifndef LFR_AST_DATADECL_H
#define LFR_AST_DATADECL_H
#include <RFintrusive_list.h>

struct ast_node;
struct parser_file;

struct ast_datadecl {
    //! identifier of the name
    struct ast_node *name;
    //! List of ast nodes that are members of the declaration
    struct RFilist_head members;
};

struct ast_node *ast_datadecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep, 
                                     struct ast_node *name);

void ast_datadecl_add_member(struct ast_node *n, struct ast_node *c);

#endif
