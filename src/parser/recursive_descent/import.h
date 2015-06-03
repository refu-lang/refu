#ifndef LFR_PARSER_IF_IMPORT_H
#define LFR_PARSER_IF_IMPORT_H

#include <stdbool.h>
#include <lexer/tokens.h>

struct parser;

#define TOKEN_IS_IMPORT(i_tok_) (                                       \
        (i_tok_) &&                                                     \
        ((i_tok_)->type == TOKEN_KW_IMPORT || (i_tok_)->type == TOKEN_KW_FOREIGN_IMPORT))

/**
 * import_statement = (import | foreign_import) identifier_list
 */
struct ast_node *parser_acc_import(struct parser *p);

#endif
