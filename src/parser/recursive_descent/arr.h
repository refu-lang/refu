#ifndef LFR_PARSER_ARRAY_H
#define LFR_PARSER_ARRAY_H

struct ast_parser;

/**
 * single_array_specificer = "[" constant_expression "]"
 * array_specifier' = single_aray_specifier 
 *                  / EMPTY
 * array_specificer = single_array_specifier array_specifier'
 */
struct ast_node *ast_parser_acc_arrspec(struct ast_parser *p);
#endif
