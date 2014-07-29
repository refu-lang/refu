#include <RFintrusive_list.h>
#include <RFstring.h>

struct ast_location {
    struct RFstring *file_name;
    unsigned int start_line;
    unsigned int start_col;
    unsigned int end_line;
    unsigned int end_col;
};


enum ast_type {
    AST_IDENTIFIER = 1,
    AST_STRING_LITERAL
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    union {
        int value;
        struct {
            struct RFilist_head children;
            unsigned int children_num;
        };
    };
};


struct ast_node *ast_node_new(enum ast_type type,
                              struct ast_location *location);


