#ifndef LFR_AST_ARRAY_REFERENCE_DECLS_H
#define LFR_AST_ARRAY_REFERENCE_DECLS_H

struct ast_node;
struct inplocation_mark;

struct ast_arrayref {
    //! identifier of the array name
    struct ast_node *name;
    //! expression of the array subscript
    struct ast_node *expr;
};
#endif
