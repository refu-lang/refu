#ifndef LFR_PARSER_DATA_H
#define LFR_PARSER_DATA_H
struct ast_node;
struct parser_file;

struct ast_node *parser_file_acc_datadecl(struct parser_file *f);
struct ast_node *parser_file_acc_datadesc(struct parser_file *f,
                                          struct ast_node *left);

#endif
