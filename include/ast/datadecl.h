#ifndef LFR_AST_DATADECL_H
#define LFR_AST_DATADECL_H
#include <RFintrusive_list.h>

struct ast_node;
struct parser_file;

enum dataop_type {
    DATAOP_SUM,
    DATAOP_PRODUCT,
    DATAOP_IMPLICATION
};

struct ast_dataop {
    enum dataop_type type;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_datadesc {
    struct ast_node *id;
    struct ast_node *desc;
};

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

void ast_datadecl_destroy(struct ast_node *n);

void ast_datadecl_add_member(struct ast_node *n, struct ast_node *c);

struct RFstring *ast_datadecl_name_str(struct ast_node *n);

void ast_datadecl_print(struct ast_node *n, int depth, const char *description);
#endif
