#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H


struct parser_file;
struct ast_node;
struct RFstring;

struct ast_node *ast_identifier_create(struct parser_file *file,
                                       char *sp, char *ep);
struct RFstring *ast_identifier_str(struct ast_node *n);
void ast_identifier_print(struct ast_node *n, int depth);
#endif
