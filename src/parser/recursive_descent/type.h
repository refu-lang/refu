#ifndef LFR_PARSER_TYPE_H
#define LFR_PARSER_TYPE_H
struct ast_node;
struct parser;

#define TYPETERM_START_STR     "'(' or identifier"
#define TYPEFACTOR_START_STR   "'(' or identifier"
#define TYPEDESC_START_STR   "'(' or identifier"

/**
 * type_description = type_term type_description'
 *
 * type_description' = TOKEN_OP_COMMA type_term type_description'
 *                   / EMPTY
 *
 * type_term = type_factor type_term'
 *
 * type_term' = TOKEN_OP_TYPESUM type_factor type_term'
 *            / EMPTY
 *
 * type_factor = TOKEN_SM_OPAREN type_description TOKEN_SM_CPAREN
 *             / identifier TOKEN_SM_COLON type_description
 *             / identifier
 *
 *
 * Set of possible first tokens for a type description
 * FIRST(type_description) = { identifier, TOKEN_SM_OPAREN }
 */
struct ast_node *parser_acc_typedesc(struct parser *p);
/**
 * type_declaration = TOKEN_KW_TYPE TOKEN_SM_OCBRACE type_description TOKEN_SM_CCBRACE
 */
struct ast_node *parser_acc_typedecl(struct parser *p);
#endif
