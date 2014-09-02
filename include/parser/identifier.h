#ifndef LFR_PARSER_IDENTIFIER_H
#define LFR_PARSER_IDENTIFIER_H

struct parser_file;

struct ast_node *parser_file_acc_identifier(struct parser_file *f);

/**
 * annotated_identifier = ["const"] identifier [generic_attributes]
 */
struct ast_node *parser_file_acc_xidentifier(struct parser_file *f);
#endif
