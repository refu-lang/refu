#ifndef LFR_AST_OPERATORS_DECLS_H
#define LFR_AST_OPERATORS_DECLS_H

struct ast_node;
struct type;

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
    //! The type of binary operation performed
    enum binaryop_type type;
    //! The left opeand ast node
    struct ast_node *left;
    //! The right operand ast node
    struct ast_node *right;
    //! [Optional] May only exist after typechecking
    //! The common type to which both operands can be typecast in the backend
    const struct type *common_type;
};

enum unaryop_type {
    UNARYOP_AMPERSAND,
    UNARYOP_INC,
    UNARYOP_DEC,
    UNARYOP_MINUS,
    UNARYOP_PLUS,
};

struct ast_unaryop {
    enum unaryop_type type;
    struct ast_node *operand;
};

#endif
