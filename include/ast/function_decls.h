#ifndef LFR_AST_FUNCTIONS_DECLS_H
#define LFR_AST_FUNCTIONS_DECLS_H

struct ast_node;
struct inplocation_mark;

struct ast_fndecl {
    //! identifier of the name
    struct ast_node *name;
    //! Optional: generic declaration
    struct ast_node *genr;
    //! type description of the arguments
    struct ast_node *args;
    //! Optional: type description of the return value
    struct ast_node *ret;
};


struct ast_fnimpl {
    //! The function's declaration (signature)
    struct ast_node *decl;
    //! The function's body
    struct ast_node *body;
};

struct ast_fncall {
    //! identifier of the name
    struct ast_node *name;
    //! Optional: generic attribute
    struct ast_node *genr;
};
#endif
