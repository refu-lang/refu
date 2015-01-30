#ifndef LFR_AST_OPERATORS_DECLS_H
#define LFR_AST_OPERATORS_DECLS_H

struct ast_node;

enum binaryop_type {
    BINARYOP_ADD,
    BINARYOP_SUB,
    BINARYOP_MUL,
    BINARYOP_DIV,

    BINARYOP_CMP_EQ,
    BINARYOP_CMP_NEQ,
    BINARYOP_CMP_GT,
    BINARYOP_CMP_GTEQ,
    BINARYOP_CMP_LT,
    BINARYOP_CMP_LTEQ,

    BINARYOP_LOGIC_AND,
    BINARYOP_LOGIC_OR,

    BINARYOP_ARRAY_REFERENCE,
    BINARYOP_MEMBER_ACCESS,

    BINARYOP_BITWISE_OR,
    BINARYOP_BITWISE_AND,
    BINARYOP_BITWISE_XOR,

    BINARYOP_ASSIGN,
    BINARYOP_COMMA
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
