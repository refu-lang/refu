#ifndef LFR_PARSER_GENERICS_H
#define LFR_PARSER_GENERICS_H

#include <stdbool.h>

struct ast_node;
struct ast_parser;

#define GENRATTR_START_COND(tok_)               \
    ((tok_) && (tok_)->type == TOKEN_OP_LT)

#define GENRDECL_START_COND(tok_)               \
    ((tok_) && (tok_)->type == TOKEN_OP_LT)

/**
 * generic_declaration = "<" generic_decls ">"
 *
 * generic_decls = generic_decl_single generic_decls'
 *
 * generic_decls' = TOKEN_OP_COMMA generic_decl_single generic_decls'
 *                / EMPTY
 *
 * generic_decl_single = "Type" identifier
 */
struct ast_node *ast_parser_acc_genrdecl(struct ast_parser *p);

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
struct ast_node *ast_parser_acc_genrattr(struct ast_parser *p, bool expect_it);
#endif
