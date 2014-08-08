#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H


struct parser_file;
struct ast_node;

struct ast_node *ast_identifier_create(struct parser_file *file,
                                       char *sp, char *ep);
                                       

#endif
