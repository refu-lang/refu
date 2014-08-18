#ifndef LFR_PARSER_FUNCTION_H
#define LFR_PARSER_FUNCTION_H

#include <stdbool.h>

struct ast_node;
struct parser_file;
struct RFilist_head;

bool parser_file_acc_commsep_args(struct parser_file *f,
                                  struct RFilist_head *args);
struct ast_node *parser_file_acc_fndecl(struct parser_file *f);

#endif
