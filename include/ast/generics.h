#ifndef LFR_AST_GENERICS_H
#define LFR_AST_GENERICS_H

struct ast_node;
struct parser_file;

struct ast_genrtype {
    struct ast_node *type;
    struct ast_node *id;
};

struct ast_node *ast_genrtype_create(struct parser_file *f, char *sp, char *ep,
                                     struct ast_node *type, struct ast_node *id);

struct ast_node *ast_genrdecl_create(struct parser_file *f, char *sp, char *ep);

struct ast_node *ast_genrattr_create(struct parser_file *f, char *sp, char *ep);
#endif
