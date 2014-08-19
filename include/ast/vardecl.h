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

struct RFstring *ast_vardecl_name_str(struct ast_node *n);
struct RFstring *ast_vardecl_type_str(struct ast_node *n);

void ast_vardecl_print(struct ast_node *n, int depth, const char *description);
#endif
