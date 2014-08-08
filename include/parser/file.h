#ifndef LFR_PARSER_FILE_H
#define LFR_PARSER_FILE_H

#include <RFstring.h>
#include <RFintrusive_list.h>

#include <parser/string.h>
#include <parser/offset.h>

struct ast_node;

struct parser_file {
    struct RFstring file_name;
    struct parser_string pstr;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
    char *bp;
    struct parser_offset offset;
    struct ast_node *root;
};

struct parser_file *parser_file_new(const struct RFstring *name);
void parser_file_deinit(struct parser_file *f);

#endif
