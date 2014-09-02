#ifndef LFR_PARSER_GENERICS_H
#define LFR_PARSER_GENERICS_H

struct ast_node;
struct parser_file;

/**
 * generic_decl_single = identifier identifier
 *
 * generic_decls = generic_decl_single
 *               / generic_decls "," generic_decl_single
 *
 * generic_declaration = "<" generic_decls ">"
 */
struct ast_node *parser_file_acc_genrdecl(struct parser_file *f);

/**
 * generic_attribute_single = "(" type_description ")"
 *                          / annotated_identifier
 *
 * generic_attribute = generic_attribute_single
 *                   / generic_atribute "," generic_atribute_single
 *
 * generic_attributes = "<" generic_attribute ">"
 */
struct ast_node *parser_file_acc_genrattr(struct parser_file *f);

#endif
