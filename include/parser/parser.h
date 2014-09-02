#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <parser/offset.h>
#include <parser/file.h>

struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
};


struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                         const struct RFstring *name);

void parser_flush_messages(struct parser_ctx *parser);


struct ast_node *parser_file_acc_vardecl(struct parser_file *f);
#endif
