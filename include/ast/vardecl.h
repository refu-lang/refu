#ifndef LFR_AST_VARDECL_H
#define LFR_AST_VARDECL_H
struct ast_node;
struct inplocation_mark;

struct ast_vardecl {
    //! identifier of the name
    struct ast_node *name;
    //! identifier of the type
    struct ast_node *type;
};

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *name,
                                    struct ast_node *type);

struct RFstring *ast_vardecl_name_str(struct ast_node *n);
struct RFstring *ast_vardecl_type_str(struct ast_node *n);
#endif
