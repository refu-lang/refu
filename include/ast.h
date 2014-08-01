#include <RFintrusive_list.h>
#include <RFstring.h>

struct parser_file;

struct ast_location {
    struct parser_file *file;
    unsigned int start_line;
    unsigned int start_col;
    unsigned int end_line;
    unsigned int end_col;
    char *beg;
    char *end;
};


enum ast_type {
    AST_ROOT,
    AST_BLOCK,
    AST_IDENTIFIER,
    AST_STRING_LITERAL,
    AST_VARIABLE_DECLARATION
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    union {
        struct RFstring value_identifier;
        struct {
            struct RFilist_head children;
            unsigned int children_num;
        };
    };
};


struct ast_node *ast_node_new(enum ast_type type,
                              struct parser_file pfile,
                              char *sp, char *ep);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);


