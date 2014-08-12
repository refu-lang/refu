#ifndef LFR_PARSER_H
#define LFR_PARSER_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <parser/offset.h>
#include <parser/file.h>

struct info_ctx;
struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
    struct info_ctx *info;
};


struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                         const struct RFstring *name);

#endif
