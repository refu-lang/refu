#include "expression.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/operators.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"
#include "vardecl.h"
#include "function.h"
#include "ifexpr.h"

#define MAX_LEVEL_OP_PRECEDENCE 13

static struct ast_node *parser_acc_expression_prime(
    struct parser *p,
    struct ast_node *left_hand_side,
    int level);

static struct ast_node *parser_acc_expr_element(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *tok2;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok) {
        return NULL;
    }
    tok2 = lexer_lookahead(p->lexer, 2);

    // check for a parenthesized expression
    if (tok->type == TOKEN_SM_OPAREN) {
        //consume parentheses opening
        lexer_next_token(p->lexer);

        n = parser_acc_expression(p);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected "EXPR_ELEMENT_START" after '('");
            return NULL;
        }

        tok = lexer_next_token(p->lexer);
        if (!tok || tok->type != TOKEN_SM_CPAREN) {
            parser_synerr(p, ast_node_endmark(n), NULL,
                          "expected ')' after expression");
            return NULL;
        }

        return n;
    } else if (TOKENS_ARE_POSSIBLE_VARDECL(tok, tok2)) {
        n = parser_acc_vardecl(p);
        if (!n) {
            parser_synerr(p, token_get_end(tok), NULL,
                          "expected a variable declaration");
            return NULL;
        }
        return n;
    } else if (TOKENS_ARE_POSSIBLE_FNCALL(tok, tok2) && (n = parser_acc_fncall(p, false))) {
        return n;
    } else if (TOKEN_IS_POSSIBLE_IFEXPR(tok)) {
        n = parser_acc_ifexpr(p, TOKEN_KW_IF);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected an if expression");
            return NULL;
        }
        return n;
    } else if (TOKEN_IS_NUMERIC_CONSTANT(tok) ||
        tok->type == TOKEN_IDENTIFIER ||
        tok->type == TOKEN_STRING_LITERAL) {
        n = token_get_value(tok);
    } else {
        // no expression found. This is not an error
        return NULL;
    }

    //consume the token and return
    lexer_next_token(p->lexer);
    return n;
}

#define TOKEN_IS_PREFIX_UNARY_OP(tok_)              \
    (tok && ((tok_)->type == TOKEN_OP_AMPERSAND ||  \
             (tok_)->type == TOKEN_OP_INC ||        \
             (tok_)->type == TOKEN_OP_DEC))

#define TOKEN_IS_POSTFIX_UNARY_OP(tok_)         \
    (tok && ((tok_)->type == TOKEN_OP_INC ||    \
             (tok_)->type == TOKEN_OP_DEC))

static struct ast_node *parser_acc_exprfactor(struct parser *p)
{
    struct ast_node *element;
    struct ast_node *op;
    struct token *tok;
    struct token *prefix = NULL;

    //check for prefix unary operators
    tok = lexer_lookahead(p->lexer, 1);
    if (TOKEN_IS_PREFIX_UNARY_OP(tok)) {
        prefix = tok;
        //consume the prefix unary operator
        lexer_next_token(p->lexer);
    }

    element = parser_acc_expr_element(p);
    if (!element) {
        if (prefix) {
            parser_synerr(p, token_get_end(prefix), NULL,
                          "Expected "EXPR_ELEMENT_START" after \""
                          ""RF_STR_PF_FMT"\"",
                          RF_STR_PF_ARG(tokentype_to_str(prefix->type)));
        }
        return NULL;
    }
    if (prefix) {
        op = ast_unaryop_create(token_get_start(prefix),
                                ast_node_endmark(element),
                                unaryop_type_from_token(prefix), element);
        if (!op) {
            RF_ERRNOMEM();
            return NULL;
        }
        return op;
    }

    //check for a postfix unary operator
    tok = lexer_lookahead(p->lexer, 1);
    if (TOKEN_IS_POSTFIX_UNARY_OP(tok)) {
        //consume the prefix unary operator
        lexer_next_token(p->lexer);

        op = ast_unaryop_create(ast_node_startmark(element),
                                token_get_end(tok),
                                unaryop_type_from_token(tok), element);
        if (!op) {
            RF_ERRNOMEM();
            return NULL;
        }
        return op;
    }

    // else no unary ops, it's just the expression element
    return element;
}

static inline bool check_operator_type(struct token *tok, int level)
{
    if (!tok) {
        return false;
    }

    switch(level) {
    case 1: /* comma operator */
        return (tok->type == TOKEN_OP_COMMA);
    case 2: /* assignment */
        return (tok->type == TOKEN_OP_ASSIGN);
    case 3: /* logic OR */
        return (tok->type == TOKEN_OP_LOGIC_OR);
    case 4: /* logic AND */
        return (tok->type == TOKEN_OP_LOGIC_AND);
    case 5: /* bitwise OR */
        return (tok->type == TOKEN_OP_TYPESUM);
    case 6: /* bitwise XOR */
        return (tok->type == TOKEN_OP_BITWISE_XOR);
    case 7: /* bitwise AND */
        return (tok->type == TOKEN_OP_AMPERSAND);



    case 8: /* equality comparison */
        return (tok->type == TOKEN_OP_EQ || tok->type == TOKEN_OP_NEQ);
    case 9: /* relational comparison */
        return (tok->type == TOKEN_OP_GT   ||
                tok->type == TOKEN_OP_GTEQ ||
                tok->type == TOKEN_OP_LT   ||
                tok->type == TOKEN_OP_LTEQ);

    case 10: /* additive operators */
        return (tok->type == TOKEN_OP_PLUS || tok->type == TOKEN_OP_MINUS);
    case 11: /* multiplicative operators */
        return (tok->type == TOKEN_OP_MULTI || tok->type == TOKEN_OP_DIV);
    case 12: /* access operators */
        return tok->type == TOKEN_OP_MEMBER_ACCESS || tok->type == TOKEN_SM_OSBRACE;
    /* case 12: */
    /*     return tok->type == TOKEN_SM_OSBRACE; */
    /* case 13: */
    /*     return tok->type == TOKEN_OP_MEMBER_ACCESS; */
    case MAX_LEVEL_OP_PRECEDENCE: /* no operators at the last level (expr_factor) */
        return false;
    }

    // illegal expression parsing level, should never happen
    RF_ASSERT_OR_CRITICAL(false,
                          "Illegal level %d for expression parsing", level);
    return false;
}

static struct ast_node *parser_acc_exprlevel(struct parser *p, int level)
{
    struct ast_node *prime;
    struct ast_node *term;

    if (level == MAX_LEVEL_OP_PRECEDENCE) { // end, we got to the factor level
        term = parser_acc_exprfactor(p);
    } else {
        term = parser_acc_exprlevel(p, level + 1);
    }
    if (!term) {
        return NULL;
    }

    prime = parser_acc_expression_prime(p, term, level);
    if (parser_has_syntax_error(p)) {
        return NULL;
    }

    return prime ? prime : term;
}

static struct ast_node *parser_acc_expression_prime(
    struct parser *p,
    struct ast_node *left_hand_side,
    int level)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;


    tok = lexer_lookahead(p->lexer, 1);
    if (!check_operator_type(tok, level)) {
        return NULL;
    }
    //consume operator
    lexer_next_token(p->lexer);

    op = ast_binaryop_create(ast_node_startmark(left_hand_side), NULL,
                             binaryop_type_from_token(tok),
                             left_hand_side, NULL);
    if (!op) {
        RF_ERRNOMEM();
        return NULL;
    }
    right_hand_side = parser_acc_exprlevel(p, level + 1);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected "EXPR_ELEMENT_START" after "
                      "\""RF_STR_PF_FMT"\"",
                      RF_STR_PF_ARG(tokentype_to_str(tok->type)));
        return NULL;
    }
    ast_binaryop_set_right(op, right_hand_side);
    // special case here for array reference operator we need to consume the closing bracket
    if (ast_binaryop_op(op) == BINARYOP_ARRAY_REFERENCE) {
        tok = lexer_lookahead(p->lexer, 1);
        if (tok->type != TOKEN_SM_CSBRACE) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "Expected ']' after "RF_STR_PF_FMT,
                          RF_STR_PF_ARG(ast_node_get_name_str(right_hand_side)));
            ast_node_destroy(op);
            return NULL;
        }
        // consume ']'
        lexer_next_token(p->lexer);
        ast_node_set_end(op, token_get_end(tok));
    }

    ret = parser_acc_expression_prime(p, op, level);
    if (parser_has_syntax_error(p)) {
        // no need to free op here, since it's freed inside expression' accepting
        return NULL;
    }

    return ret ? ret : op;
}

struct ast_node *parser_acc_expression(struct parser *p)
{
    return parser_acc_exprlevel(p, 1);
}
