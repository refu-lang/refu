#include "expression.h"

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/operators.h>
#include <ast/constants.h>
#include <info/info.h>
#include <parser/parser.h>

#include "common.h"
#include "identifier.h"
#include "type.h"
#include "vardecl.h"
#include "function.h"
#include "ifexpr.h"
#include "block.h"
#include "matchexpr.h"
#include "arr.h"

#define MAX_LEVEL_OP_PRECEDENCE 13

static struct ast_node *ast_parser_acc_expression_prime(
    struct ast_parser *p,
    struct ast_node *left_hand_side,
    int level
);

static struct ast_node *ast_parser_acc_expr_element(struct ast_parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *tok2;

    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!tok) {
        return NULL;
    }
    tok2 = lexer_lookahead(parser_lexer(p), 2);

    // check for a parenthesized expression
    if (tok->type == TOKEN_SM_OPAREN) {
        //consume parentheses opening
        lexer_curr_token_advance(parser_lexer(p));

        n = ast_parser_acc_expression(p);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected "EXPR_ELEMENT_START" after '('");
            return NULL;
        }

        tok = lexer_curr_token_advance(parser_lexer(p));
        if (!tok || tok->type != TOKEN_SM_CPAREN) {
            parser_synerr(p, ast_node_endmark(n), NULL,
                          "expected ')' after expression");
            return NULL;
        }

        return n;
    } else if (TOKENS_ARE_POSSIBLE_VARDECL(tok, tok2)) {
        n = ast_parser_acc_vardecl(p);
        if (!n) {
            parser_synerr(p, token_get_end(tok), NULL,
                          "expected a variable declaration");
            return NULL;
        }
        return n;
    } else if (TOKENS_ARE_POSSIBLE_FNCALL(tok, tok2) && // check if is generic function call
               (n = ast_parser_acc_fncall(p, false))) {
        return n;
    } else if (TOKENS_ARE_FNCALL(tok, tok2)) { //normal function call
        return ast_parser_acc_fncall(p, true);
    } else if (tok->type == TOKEN_SM_OSBRACE) {
        return ast_parser_acc_bracketlist(p);
    } else if (tok->type == TOKEN_SM_OCBRACE) {
        n = ast_parser_acc_block(p, true);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected a block");
            return NULL;
        }
        return n;
    } else if (TOKEN_IS_POSSIBLE_IFEXPR(tok)) {
        n = ast_parser_acc_ifexpr(p, TOKEN_KW_IF);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected an if expression");
            return NULL;
        }
        return n;
    } else if (TOKEN_IS_POSSIBLE_MATCH_EXPRESSION(tok)) {
        n = ast_parser_acc_matchexpr(p, true, true);
        if (!n) {
            parser_synerr(p, token_get_start(tok), NULL,
                          "expected a match expression");
            return NULL;
        }
        return n;
    } else if (tok->type == TOKEN_KW_TRUE || tok->type == TOKEN_KW_FALSE) {
        n = ast_constant_create_boolean_from_tok(tok);
        if (!n) {
            RF_ERROR("Failure to create a constan boolean node");
            return NULL;
        }
    } else if (token_is_numeric_constant(tok) ||
        tok->type == TOKEN_IDENTIFIER ||
        tok->type == TOKEN_STRING_LITERAL) {
        n = lexer_token_get_value(parser_lexer(p), tok);
    } else {
        // no expression found. This is not an error
        return NULL;
    }

    //consume the token and return
    lexer_curr_token_advance(parser_lexer(p));
    return n;
}

static struct ast_node *ast_parser_acc_exprfactor(struct ast_parser *p)
{
    struct ast_node *element;
    struct ast_node *op;
    struct token *tok;
    struct token *prefix = NULL;

    //check for prefix unary operators
    tok = lexer_lookahead(parser_lexer(p), 1);
    if (tok && token_is_prefix_unaryop(tok)) {
        prefix = tok;
        //consume the prefix unary operator
        lexer_curr_token_advance(parser_lexer(p));
    }

    element = ast_parser_acc_expr_element(p);
    if (!element) {
        if (prefix) {
            parser_synerr(
                p, token_get_end(prefix), NULL,
                "Expected "EXPR_ELEMENT_START" after \""RFS_PF"\"",
                RFS_PA(tokentype_to_str(prefix->type))
            );
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
    tok = lexer_lookahead(parser_lexer(p), 1);
    if (tok && token_is_postfix_unaryop(tok)) {
        //consume the prefix unary operator
        lexer_curr_token_advance(parser_lexer(p));

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
    RF_ASSERT_OR_CRITICAL(false, return false,
                          "Illegal level %d for expression parsing", level);
}

static struct ast_node *ast_parser_acc_exprlevel(struct ast_parser *p, int level)
{
    struct ast_node *prime;
    struct ast_node *term;

    if (level == MAX_LEVEL_OP_PRECEDENCE) { // end, we got to the factor level
        term = ast_parser_acc_exprfactor(p);
    } else {
        term = ast_parser_acc_exprlevel(p, level + 1);
    }
    if (!term) {
        return NULL;
    }

    prime = ast_parser_acc_expression_prime(p, term, level);
    if (ast_parser_has_syntax_error(p)) {
        return NULL;
    }

    return prime ? prime : term;
}

static struct ast_node *ast_parser_acc_expression_prime(
    struct ast_parser *p,
    struct ast_node *left_hand_side,
    int level
)
{
    struct token *tok;
    struct ast_node *op;
    struct ast_node *right_hand_side;
    struct ast_node *ret;


    tok = lexer_lookahead(parser_lexer(p), 1);
    if (!check_operator_type(tok, level)) {
        return NULL;
    }
    //consume operator
    lexer_curr_token_advance(parser_lexer(p));

    op = ast_binaryop_create(
        ast_node_startmark(left_hand_side),
        NULL,
        binaryop_type_from_token(tok),
        left_hand_side,
        NULL
    );
    if (!op) {
        RF_ERRNOMEM();
        return NULL;
    }
    right_hand_side = ast_parser_acc_exprlevel(p, level + 1);
    if (!right_hand_side) {
        parser_synerr
            (p, token_get_end(tok), NULL,
             "Expected "EXPR_ELEMENT_START" after \""RFS_PF"\"",
             RFS_PA(tokentype_to_str(tok->type))
            );
        ast_node_destroy(op);
        return NULL;
    }
    ast_binaryop_set_right(op, right_hand_side);
    // special case here for array reference operator we need to consume the closing bracket
    if (ast_binaryop_op(op) == BINARYOP_ARRAY_REFERENCE) {
        tok = lexer_lookahead(parser_lexer(p), 1);
        if (tok->type != TOKEN_SM_CSBRACE) {
            parser_synerr(
                p, token_get_start(tok), NULL,
                "Expected ']' after "RFS_PF,
                RFS_PA(ast_node_get_name_str(right_hand_side))
            );
            ast_node_destroy(op);
            return NULL;
        }
        // consume ']'
        lexer_curr_token_advance(parser_lexer(p));
        ast_node_set_end(op, token_get_end(tok));
    }

    ret = ast_parser_acc_expression_prime(p, op, level);
    if (ast_parser_has_syntax_error(p)) {
        // no need to free op here, since it's freed inside expression' accepting
        return NULL;
    }

    return ret ? ret : op;
}

struct ast_node *ast_parser_acc_expression(struct ast_parser *p)
{
    lexer_push(parser_lexer(p));
    struct ast_node *ret = ast_parser_acc_exprlevel(p, 1);
    if (!ret && ast_parser_has_syntax_error(p)) {
        lexer_rollback(parser_lexer(p));
    } else {
        lexer_pop(parser_lexer(p));
    }
    return ret;
}
