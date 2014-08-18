#ifndef LFR_AST_GENERIC_DECL_H
#define LFR_AST_GENERIC_DECL_H
#include <RFintrusive_list.h>

struct ast_node;
struct parser_file;

/* Got only one generic type, type now */
enum genrtype {
    AST_GENR_TYPE = 0, //keyword: Type
};

struct ast_genrtype {
    enum genrtype type;
    struct ast_node *id;
    struct RFilist_node lh;
};

struct ast_genrdecl {
    struct RFilist_head members;
};

struct ast_node *ast_genrdecl_create(struct parser_file *f, char *sp, char *ep);
void ast_genrdecl_destroy(struct ast_node *n);

bool ast_genrdecl_add_member(struct ast_node *n,
                             enum genrtype type,
                             struct ast_node *c);


#endif
