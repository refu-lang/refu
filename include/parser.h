#include <RFintrusive_list.h>
#include <RFstring.h>

struct parser_file {
    struct RFstring file_name;
    struct RFstringx buffer;
    struct RFilist_node lh;
    unsigned int current_line;
    unsigned int current_col;
};

struct parser_ctx {
    struct RFilist_head files;
    struct parser_file *current_file;
};


struct parser_ctx *parser_new();
bool parser_process_file(struct parser_ctx *parser,
                        const struct RFstring *name);
