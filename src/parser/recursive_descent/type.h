#ifndef LFR_PARSER_TYPE_H
#define LFR_PARSER_TYPE_H
struct ast_node;
struct parser;

#define TYPETERM_START_STR     "'(' or identifier"
#define TYPEFACTOR_START_STR   "'(' or identifier"
#define TYPEELEMENT_START_STR   "'(' or identifier"
#define TYPEDESC_START_STR   "'(' or identifier"

#define TYPEDESC_START_COND(tok_)                                       \
    ((tok_) &&                                                          \
     ((tok_)->type == TOKEN_SM_OPAREN || (tok_)->type == TOKEN_IDENTIFIER))

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
 * type_factor = type_element type_factor'
 *
 * type_factor' = TOKEN_OP_IMPL type_element type_factor'
 *              / EMPTY
 * type_element = TOKEN_SM_OPAREN type_description TOKEN_SM_CPAREN
 *              / identifier TOKEN_SM_COLON annotated_identifier
 *              / annotated_identifier
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
