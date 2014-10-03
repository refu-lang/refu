#include "expression.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/operators.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"
#include "function.h"
#include "arrayref.h"


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
    } else if (TOKENS_ARE_POSSIBLE_ARRAYREF(tok, tok2)) {
        n = parser_acc_arrayref(p);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected array reference");
            return NULL;
        }
        return n;
    } else if (TOKENS_ARE_POSSIBLE_FNCALL(tok, tok2)) {
        // function call
        n = parser_acc_fncall(p);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected function call");
            return NULL;
        }
        return n;

    } else if (TOKEN_IS_NUMERIC_CONSTANT(tok) ||
        tok->type == TOKEN_IDENTIFIER ||
        tok->type == TOKEN_STRING_LITERAL) {
        n = token_get_value(tok);
    } else {
        parser_synerr(p, token_get_start(tok), NULL,
                      "expected "EXPR_ELEMENT_START);
        return NULL;
    }

    //consume the token and return
    lexer_next_token(p->lexer);
    return n;
}

#define TOKEN_IS_PREFIX_UNARY_OP(tok_)          \
    (tok && ((tok_)->type == TOKEN_OP_AMPERSAND ||  \
             (tok_)->type == TOKEN_OP_INC ||        \
             (tok_)->type == TOKEN_OP_DEC))

#define TOKEN_IS_POSTFIX_UNARY_OP(tok_)          \
    (tok && ((tok_)->type == TOKEN_OP_INC ||        \
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
            //TODO: bad error
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
            //TODO: bad error
            return NULL;
        }
        return op;
    }

    // else no unary ops, it's just the expression element
    return element;
}

static struct ast_node *parser_acc_exprterm_prime(
    struct parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || (tok->type != TOKEN_OP_MULTI && tok->type != TOKEN_OP_DIV)) {
        return NULL;
    }
    //consume operator
    lexer_next_token(p->lexer);

    lexer_push(p->lexer);

    op = ast_binaryop_create(ast_node_startmark(left_hand_side), NULL,
                             binaryop_type_from_token(tok),
                             left_hand_side, NULL);
    if (!op) {
        //TODO: bad error
    }

    right_hand_side = parser_acc_exprfactor(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected "EXPR_ELEMENT_START" after \""RF_STR_PF_FMT"\"",
                      RF_STR_PF_ARG(tokentype_to_str(tok->type)));
        ast_node_destroy(op);
        return NULL;
    }
    ast_binaryop_set_right(op, right_hand_side);

    ret = parser_acc_exprterm_prime(p, op);
    if (parser_has_syntax_error(p)) {
        // no need to free term, since it's freed inside exprterm' accepting
        return NULL;
    }

    return ret ? ret : op;
}

static struct ast_node *parser_acc_exprterm(struct parser *p)
{
    struct ast_node *factor;
    struct ast_node *prime;

    factor = parser_acc_exprfactor(p);
    if (!factor) {
        return NULL;
    }
    prime = parser_acc_exprterm_prime(p, factor);
    if (parser_has_syntax_error(p)) {
        // no need to free factor, since it's freed inside exprterm' accepting
        return NULL;
    }

    return (prime) ? prime : factor;
}

static struct ast_node *parser_acc_expression_prime(
    struct parser *p,
    struct ast_node *left_hand_side)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;

    tok = lexer_lookahead(p->lexer, 1);
    if (!tok || (tok->type != TOKEN_OP_PLUS && tok->type != TOKEN_OP_MINUS)) {
        return NULL;
    }
    //consume operator
    lexer_next_token(p->lexer);

    lexer_push(p->lexer);

    op = ast_binaryop_create(ast_node_startmark(left_hand_side), NULL,
                             binaryop_type_from_token(tok),
                             left_hand_side, NULL);
    if (!op) {
        //TODO: bad error
        return NULL;
    }
    right_hand_side = parser_acc_exprterm(p);
    if (!right_hand_side) {
        parser_synerr(p, token_get_end(tok), NULL,
                      "Expected "EXPR_ELEMENT_START" after "
                      "\""RF_STR_PF_FMT"\"",
                      RF_STR_PF_ARG(tokentype_to_str(tok->type)));
        ast_node_destroy(op);
        return NULL;
    }
    ast_binaryop_set_right(op, right_hand_side);

    ret = parser_acc_expression_prime(p, op);
    if (parser_has_syntax_error(p)) {
        // no need to free op, since it's freed inside expression' accepting
        return NULL;
    }

    return ret ? ret : op;
}

struct ast_node *parser_acc_expression(struct parser *p)
{
    struct ast_node *prime;
    struct ast_node *term;

    term = parser_acc_exprterm(p);
    if (!term) {
        return NULL;
    }

    prime = parser_acc_expression_prime(p, term);
    if (parser_has_syntax_error(p)) {
        return NULL;
    }

    return prime ? prime : term;
}


bool parser_acc_expressions_list(struct parser *p,
                                 struct ast_node *parent)
{
    struct ast_node *expr;
    struct token *tok;

    // check for the first expression
    expr = parser_acc_expression(p);

    if (!expr) { // empty list is valid
        return true;
    }
    ast_node_add_child(parent, expr);

    tok = lexer_lookahead(p->lexer, 1);
    while (tok && tok->type == TOKEN_OP_COMMA) {
        // consume the comma
        lexer_next_token(p->lexer);
        // get the next expression
        expr = parser_acc_expression(p);
        if (!expr) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected an expression after ','");
            return false;
        }
        ast_node_add_child(parent, expr);
        tok = lexer_lookahead(p->lexer, 1);
    }

    return true;
}
