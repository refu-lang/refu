#ifndef LFR_AST_DATADECL_H
#define LFR_AST_DATADECL_H

#include <RFintrusive_list.h>
#include <parser/tokens.h>

struct ast_node;
struct parser_file;

struct ast_datadecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
};

struct ast_node *ast_datadecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name,
                                     struct ast_node *desc);
void ast_datadecl_destroy(struct ast_node *n);


struct RFstring *ast_datadecl_name_str(struct ast_node *n);

void ast_datadecl_print(struct ast_node *n, int depth, const char *description);
#endif
