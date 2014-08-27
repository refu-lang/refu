#ifndef LFR_AST_TYPEDECL_H
#define LFR_AST_TYPEDECL_H

#include <RFintrusive_list.h>
#include <parser/tokens.h>

struct ast_node;
struct parser_file;

struct ast_typedecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
};

struct ast_node *ast_typedecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name,
                                     struct ast_node *desc);
void ast_typedecl_destroy(struct ast_node *n);


struct RFstring *ast_typedecl_name_str(struct ast_node *n);

void ast_typedecl_print(struct ast_node *n, int depth, const char *description);
#endif
