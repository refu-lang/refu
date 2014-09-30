#ifndef LFR_AST_OPERATORS_DECLS_H
#define LFR_AST_OPERATORS_DECLS_H

struct ast_node;

enum binaryop_type {
    BINARYOP_ADD,
    BINARYOP_SUB,
    BINARYOP_MUL,
    BINARYOP_DIV,
};

struct ast_binaryop {
    enum binaryop_type type;
    struct ast_node *left;
    struct ast_node *right;
};

enum unaryop_type {
    UNARYOP_AMPERSAND,
    UNARYOP_INC,
    UNARYOP_DEC,
};

struct ast_unaryop {
    enum unaryop_type type;
    struct ast_node *operand;
};

#endif
