#ifndef LFR_PARSER_GENERICS_H
#define LFR_PARSER_GENERICS_H

struct ast_node;
struct parser;

/**
 * generic_declaration = "<" generic_decls ">"
 *
 * generic_decls = generic_decl_single generic_decls'
 *
 * generic_decls' = TOKEN_OP_COMMA generic_decl_single generic_decls'
 *                / EMPTY
 *
 * generic_decl_single = identifier identifier
 */
struct ast_node *parser_acc_genrdecl(struct parser *p);

/**
 * generic_attributes = "<" generic_attribute ">"
 *
 * generic_attribute = generic_attribute_single generic_attribute'
 *
 * generic_attribute' = TOKEN_OP_COMMA generic_attribute_single generic_attribute'
 *                    / EMPTY
 *
 * generic_attribute_single = "(" type_description ")"
 *                          / annotated_identifier
 *
 */
struct ast_node *parser_acc_genrattr(struct parser *p);

#endif
