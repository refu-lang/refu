#include <ast/operators.h>

#include <lexer/tokens.h>
#include <lexer/lexer.h>

struct ast_node *ast_binaryop_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end,
                                     enum binaryop_type type,
                                     struct ast_node *left,
                                     struct ast_node *right)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_BINARY_OPERATOR, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->binaryop.type = type;
    ast_node_register_child(ret, left, binaryop.left);
    ast_node_register_child(ret, right, binaryop.right);

    return ret;
}

i_INLINE_INS void ast_binaryop_set_right(struct ast_node *op, struct ast_node *r);
i_INLINE_INS enum binaryop_type ast_binaryop_op(struct ast_node *op);
i_INLINE_INS bool ast_node_is_specific_binaryop(struct ast_node *n,
                                                 enum binaryop_type optype);

static const struct RFstring binaryop_operation_names[] = {
    [BINARYOP_ADD]               =   RF_STRING_STATIC_INIT("addition"),
    [BINARYOP_SUB]               =   RF_STRING_STATIC_INIT("subtraction"),
    [BINARYOP_MUL]               =   RF_STRING_STATIC_INIT("multiplication"),
    [BINARYOP_DIV]               =   RF_STRING_STATIC_INIT("division"),

    [BINARYOP_CMP_EQ]            =   RF_STRING_STATIC_INIT("equality"),
    [BINARYOP_CMP_NEQ]           =   RF_STRING_STATIC_INIT("unequality"),
    [BINARYOP_CMP_GT]            =   RF_STRING_STATIC_INIT("greater than comparison"),
    [BINARYOP_CMP_GTEQ]          =   RF_STRING_STATIC_INIT("greater or equal comparison"),
    [BINARYOP_CMP_LT]            =   RF_STRING_STATIC_INIT("less than comparison"),
    [BINARYOP_CMP_LTEQ]          =   RF_STRING_STATIC_INIT("less or equal comparison"),

    [BINARYOP_LOGIC_AND]         =   RF_STRING_STATIC_INIT("logical and"),
    [BINARYOP_LOGIC_OR]          =   RF_STRING_STATIC_INIT("logical or"),

    [BINARYOP_ARRAY_REFERENCE]   =   RF_STRING_STATIC_INIT("array reference"),
    [BINARYOP_MEMBER_ACCESS]     =   RF_STRING_STATIC_INIT("member access"),

    [BINARYOP_BITWISE_OR]        =   RF_STRING_STATIC_INIT("bitwise or"),
    [BINARYOP_BITWISE_AND]       =   RF_STRING_STATIC_INIT("bitwise and"),
    [BINARYOP_BITWISE_XOR]       =   RF_STRING_STATIC_INIT("biwise xor"),

    [BINARYOP_ASSIGN]            =   RF_STRING_STATIC_INIT("assignment"),
    [BINARYOP_COMMA]             =   RF_STRING_STATIC_INIT("comma")
};

const struct RFstring *ast_binaryop_operation_name_str(enum binaryop_type op)
{
    return &binaryop_operation_names[op];
}


static const enum binaryop_type  bop_type_lookup[] = {
    [TOKEN_OP_PLUS]     =   BINARYOP_ADD,
    [TOKEN_OP_MINUS]    =   BINARYOP_SUB,
    [TOKEN_OP_MULTI]    =   BINARYOP_MUL,
    [TOKEN_OP_DIV]      =   BINARYOP_DIV,

    [TOKEN_OP_EQ]       =   BINARYOP_CMP_EQ,
    [TOKEN_OP_NEQ]      =   BINARYOP_CMP_NEQ,
    [TOKEN_OP_GT]       =   BINARYOP_CMP_GT,
    [TOKEN_OP_GTEQ]     =   BINARYOP_CMP_GTEQ,
    [TOKEN_OP_LT]       =   BINARYOP_CMP_LT,
    [TOKEN_OP_LTEQ]     =   BINARYOP_CMP_LTEQ,

    [TOKEN_OP_LOGIC_AND] =   BINARYOP_LOGIC_AND,
    [TOKEN_OP_LOGIC_OR]  =   BINARYOP_LOGIC_OR,

    [TOKEN_SM_OSBRACE]        =   BINARYOP_ARRAY_REFERENCE,
    [TOKEN_OP_MEMBER_ACCESS]  =   BINARYOP_MEMBER_ACCESS,

    [TOKEN_OP_TYPESUM]   =   BINARYOP_BITWISE_OR,
    [TOKEN_OP_AMPERSAND]   =   BINARYOP_BITWISE_AND,
    [TOKEN_OP_BITWISE_XOR]   =   BINARYOP_BITWISE_XOR,

    [TOKEN_OP_ASSIGN]   =   BINARYOP_ASSIGN,
    [TOKEN_OP_COMMA]   =   BINARYOP_COMMA
};

enum binaryop_type binaryop_type_from_token(struct token *tok)
{
    RF_ASSERT(token_is_binaryop(tok), "Requested binary op type from non "
              "binaryop token type \""RF_STR_PF_FMT"\"",
              RF_STR_PF_ARG(tokentype_to_str(tok->type)));
    return bop_type_lookup[tok->type];
}

static const enum token_type  token_type_from_bop_lookup[] = {
    [BINARYOP_ADD]                =   TOKEN_OP_PLUS,
    [BINARYOP_SUB]                =   TOKEN_OP_MINUS,
    [BINARYOP_MUL]                =   TOKEN_OP_MULTI,
    [BINARYOP_DIV]                =   TOKEN_OP_DIV,

    [BINARYOP_CMP_EQ]             =   TOKEN_OP_EQ,
    [BINARYOP_CMP_NEQ]            =   TOKEN_OP_NEQ,
    [BINARYOP_CMP_GT]             =   TOKEN_OP_GT,
    [BINARYOP_CMP_GTEQ]           =   TOKEN_OP_GTEQ,
    [BINARYOP_CMP_LT]             =   TOKEN_OP_LT,
    [BINARYOP_CMP_LTEQ]           =   TOKEN_OP_LTEQ,

    [BINARYOP_LOGIC_AND]          =   TOKEN_OP_LOGIC_AND,
    [BINARYOP_LOGIC_OR]           =   TOKEN_OP_LOGIC_OR,

    [BINARYOP_ARRAY_REFERENCE]    =   TOKEN_SM_OSBRACE,
    [BINARYOP_MEMBER_ACCESS]      =   TOKEN_OP_MEMBER_ACCESS,

    [BINARYOP_BITWISE_OR]         =   TOKEN_OP_BITWISE_OR,
    [BINARYOP_BITWISE_AND]        =   TOKEN_OP_BITWISE_AND,
    [BINARYOP_BITWISE_XOR]        =   TOKEN_OP_BITWISE_XOR,

    [BINARYOP_ASSIGN]             =   TOKEN_OP_ASSIGN,
    [BINARYOP_COMMA]              =   TOKEN_OP_COMMA
};

const struct RFstring *ast_binaryop_opstr(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return tokentype_to_str(token_type_from_bop_lookup[op->binaryop.type]);
}

i_INLINE_INS struct ast_node *ast_binaryop_left(struct ast_node *op);
i_INLINE_INS struct ast_node *ast_binaryop_right(struct ast_node *op);
i_INLINE_INS const struct type *ast_binaryop_common_type(struct ast_node *op);

/* -- unary operator related functions -- */

struct ast_node *ast_unaryop_create(const struct inplocation_mark *start,
                                    const struct inplocation_mark *end,
                                    enum unaryop_type type,
                                    struct ast_node *operand)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_UNARY_OPERATOR, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->unaryop.type = type;
    ast_node_register_child(ret, operand, unaryop.operand);

    return ret;
}

static const enum unaryop_type  uop_type_lookup[] = {
    [TOKEN_OP_AMPERSAND]  =   UNARYOP_AMPERSAND,
    [TOKEN_OP_INC]        =   UNARYOP_INC,
    [TOKEN_OP_DEC]        =   UNARYOP_DEC,
    [TOKEN_OP_MINUS]      =   UNARYOP_MINUS,
    [TOKEN_OP_PLUS]       =   UNARYOP_PLUS,
};

static const enum token_type  token_type_from_uop_lookup[] = {
    [UNARYOP_AMPERSAND]          =   TOKEN_OP_AMPERSAND,
    [UNARYOP_INC]                =   TOKEN_OP_INC,
    [UNARYOP_DEC]                =   TOKEN_OP_DEC,
    [UNARYOP_MINUS]              =   TOKEN_OP_MINUS,
    [UNARYOP_PLUS]               =   TOKEN_OP_PLUS
};

i_INLINE_INS enum unaryop_type ast_unaryop_op(const struct ast_node *op);
i_INLINE_INS struct ast_node *ast_unaryop_operand(struct ast_node *op);

static const struct RFstring unaryop_operation_names[] = {
    [UNARYOP_AMPERSAND]          =   RF_STRING_STATIC_INIT("addressof"),
    [UNARYOP_INC]                =   RF_STRING_STATIC_INIT("increment"),
    [UNARYOP_DEC]                =   RF_STRING_STATIC_INIT("decrement"),
    [UNARYOP_MINUS]              =   RF_STRING_STATIC_INIT("negative"),
    [UNARYOP_PLUS]               =   RF_STRING_STATIC_INIT("positive"),
};

const struct RFstring *ast_unaryop_operation_name_str(enum unaryop_type op)
{
    return &unaryop_operation_names[op];
}

const struct RFstring *ast_unaryop_opstr(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_UNARY_OPERATOR);
    return tokentype_to_str(token_type_from_uop_lookup[op->unaryop.type]);
}

enum unaryop_type unaryop_type_from_token(struct token *tok)
{
    RF_ASSERT(token_is_unaryop(tok), "Requested unary op type from non "
              "unary token type \""RF_STR_PF_FMT"\"",
              RF_STR_PF_ARG(tokentype_to_str(tok->type)));
    return uop_type_lookup[tok->type];
}
