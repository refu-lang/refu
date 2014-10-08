#ifndef LFR_PARSER_TYPECLASS_H
#define LFR_PARSER_TYPECLASS_H

struct ast_node;
struct parser;

/**
 * typeclass = TOKEN_KW_TYPECLASS identifier [generic_declaration]
 * TOKEN_SM_OCBRACE functions_declarations TOKEN_SM_CCBRACE
 */
struct ast_node *parser_acc_typeclass(struct parser *p);

/**
 * typeinstance = TOKEN_KW_INSTANCE identifier identifier [generic_declararion]
 *                TOKEN_SM_OCBRACE function_implementations TOKEN_SM_CCBRACE
 */
struct ast_node *parser_acc_typeinstance(struct parser *p);
#endif
