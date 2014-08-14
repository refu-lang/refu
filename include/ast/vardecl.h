#ifndef LFR_AST_VARDECL_H
#define LFR_AST_VARDECL_H
struct ast_node;
struct parser_file;

struct ast_vardecl {
    //! identifier of the name
    struct ast_node *name;
    //! identifier of the type
    struct ast_node *type;
};

struct ast_node *ast_vardecl_create(struct parser_file *f,
                                    char *sp,
                                    char *ep, 
                                    struct ast_node *name,
                                    struct ast_node *type);

#endif
