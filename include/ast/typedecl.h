#ifndef LFR_AST_TYPEDECL_H
#define LFR_AST_TYPEDECL_H

#include <RFintrusive_list.h>

struct ast_node;
struct inplocation_mark;

struct ast_typedecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
};

struct ast_node *ast_typedecl_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
                                     struct ast_node *name,
                                     struct ast_node *desc);

struct RFstring *ast_typedecl_name_str(struct ast_typedecl *t);
#endif
