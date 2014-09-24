#ifndef LFR_PARSER_TYPECLASS_H
#define LFR_PARSER_TYPECLASS_H

struct ast_node;
struct parser;


/**
 * type_class = TOKEN_KW_TYPECLASS identifier [generic_declaration]
 * TOKEN_SM_OCBRACE functions_declarations TOKEN_SM_CCBRACE
 */
struct ast_node *parser_acc_typeclass(struct parser *p);
#endif
