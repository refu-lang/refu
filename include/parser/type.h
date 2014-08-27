#ifndef LFR_PARSER_TYPE_H
#define LFR_PARSER_TYPE_H
struct ast_node;
struct parser_file;

struct ast_node *parser_file_acc_typedecl(struct parser_file *f);
struct ast_node *parser_file_acc_typedesc(struct parser_file *f,
                                          int *paren_count);

#endif
