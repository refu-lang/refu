#ifndef LFR_PARSER_MODULE_H
#define LFR_PARSER_MODULE_H

#include <stdbool.h>
#include <lexer/tokens.h>

struct ast_parser;

#define TOKEN_IS_IMPORT(i_tok_) (                                       \
        (i_tok_) &&                                                     \
        ((i_tok_)->type == TOKEN_KW_IMPORT || (i_tok_)->type == TOKEN_KW_FOREIGN_IMPORT))

#define TOKEN_IS_MODULE_START(tok_) ((tok_) && (tok_)->type == TOKEN_KW_MODULE)

/**
 * import_statement = (import | foreign_import) identifier_list
 */
struct ast_node *ast_parser_acc_import(struct ast_parser *p);

/**
 * module_statements = import_statement
 *                   / function_implementation
 *                   / function_declaration
 *                   / type_declaration
 *                   / NULL
 *
 * module_block = TOKEN_SM_OCBRACE module_statements TOKEN_SM_CCBRACE
 *
 * module_args = TOKEN_KW_OPAREN type_description TOKEN_KW_CPAREN
 *             / NULL
 *
 * module = TOKEN_KW_MODULE identifier module_args module_block
 */
struct ast_node *ast_parser_acc_module(struct ast_parser *p);

#endif
