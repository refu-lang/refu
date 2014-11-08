#include <ast/operators.h>

#include <lexer/tokens.h>
#include <lexer/lexer.h>

struct ast_node *ast_binaryop_create(struct inplocation_mark *start,
                                     struct inplocation_mark *end,
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

    [TOKEN_OP_LOGICAND] =   BINARYOP_LOGIC_AND,
    [TOKEN_OP_LOGICOR]  =   BINARYOP_LOGIC_OR,

    [TOKEN_OP_ASSIGN]   =   BINARYOP_ASSIGN
};

enum binaryop_type binaryop_type_from_token(struct token *tok)
{
    RF_ASSERT(TOKEN_IS_BINARY_OP(tok), "Requested binary op type from non "
              "binaryop token type \""RF_STR_PF_FMT"\"",
              RF_STR_PF_ARG(tokentype_to_str(tok->type)));
    return bop_type_lookup[tok->type];
}

static const enum token_type  token_type_lookup[] = {
    [BINARYOP_ADD]        =   TOKEN_OP_PLUS,
    [BINARYOP_SUB]        =   TOKEN_OP_MINUS,
    [BINARYOP_MUL]        =   TOKEN_OP_MULTI,
    [BINARYOP_DIV]        =   TOKEN_OP_DIV,

    [BINARYOP_CMP_EQ]     =   TOKEN_OP_EQ,
    [BINARYOP_CMP_NEQ]    =   TOKEN_OP_NEQ,
    [BINARYOP_CMP_GT]     =   TOKEN_OP_GT,
    [BINARYOP_CMP_GTEQ]   =   TOKEN_OP_GTEQ,
    [BINARYOP_CMP_LT]     =   TOKEN_OP_LT,
    [BINARYOP_CMP_LTEQ]   =   TOKEN_OP_LTEQ,

    [BINARYOP_LOGIC_AND]  =   TOKEN_OP_LOGICAND,
    [BINARYOP_LOGIC_OR]   =   TOKEN_OP_LOGICOR,

    [BINARYOP_ASSIGN]  =   TOKEN_OP_ASSIGN
};

const struct RFstring * ast_binaryop_opstr(struct ast_node *op)
{
    AST_NODE_ASSERT_TYPE(op, AST_BINARY_OPERATOR);
    return tokentype_to_str(token_type_lookup[op->binaryop.type]);
}

i_INLINE_INS struct ast_node *ast_binaryop_left(struct ast_node *op);
i_INLINE_INS struct ast_node *ast_binaryop_right(struct ast_node *op);

/* -- unary operator related functions -- */

struct ast_node *ast_unaryop_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
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
    [TOKEN_OP_DEC]        =   UNARYOP_DEC
};

enum unaryop_type unaryop_type_from_token(struct token *tok)
{
    RF_ASSERT(TOKEN_IS_UNARY_OP(tok), "Requested unary op type from non "
              "unary token type \""RF_STR_PF_FMT"\"",
              RF_STR_PF_ARG(tokentype_to_str(tok->type)));
    return uop_type_lookup[tok->type];
}
