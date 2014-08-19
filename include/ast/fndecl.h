#ifndef LFR_AST_FNDECL_H
#define LFR_AST_FNDECL_H

#include <RFintrusive_list.h>

struct ast_node;
struct parser_file;

struct ast_fndecl {
    //! identifier of the name
    struct ast_node *name;
    //! List of ast nodes that are function arguments
    struct RFilist_head args;
    //! Optional: return type of the function
    struct ast_node *ret;
    //! Optional: generic declaration
    struct ast_node *genr;
};

struct ast_node *ast_fndecl_create(struct parser_file *f,
                                   char *sp,
                                   char *ep, 
                                   struct ast_node *name);

void ast_fndecl_destroy(struct ast_node *n);

void ast_fndecl_add_arg(struct ast_node *n, struct ast_node *c);
void ast_fndecl_set_ret(struct ast_node *n, struct ast_node *r);
void ast_fndecl_set_genr(struct ast_node *n, struct ast_node *g);

struct RFstring *ast_fndecl_name_str(struct ast_node *n);
struct RFstring *ast_fndecl_ret_str(struct ast_node *n);

void ast_fndecl_print(struct ast_node *n, int depth, const char *desc);
#endif
