#ifndef LFR_PARSER_FUNCTION_H
#define LFR_PARSER_FUNCTION_H

struct parser;
struct ast_node;

/**
 * function_return = TOKEN_OP_IMPL type_description
 *
 * function_declaration = TOKEN_KW_FUNCTION identifier [generic_declaration]
 * TOKEN_SM_OPAREN type_description TOKEN_SM_CPAREN [function_return]
 */
struct ast_node *parser_acc_fndecl(struct parser *p);

#endif
