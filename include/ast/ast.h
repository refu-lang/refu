#ifndef LFR_AST_H
#define LFR_AST_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <ast/location.h>

enum ast_type {
    AST_ROOT = 0,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,

    /* from this value and up all types should have no children */
    AST_LEAVES,
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    union {
        struct RFstring identifier;
        struct {
            struct RFilist_head children;
            unsigned int children_num;
        };
    };
};

struct ast_node *ast_node_create(enum ast_type type,
                                         struct parser_file *f,
                                         char *sp, char *ep);

//will probably go away if not used
struct ast_node *ast_node_create_fromloc(enum ast_type type,
                                         struct ast_location *loc);

void ast_node_destroy(struct ast_node *n);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);

const struct RFstring *ast_node_str(struct ast_node *n);

// temporary function, to visualize an ast tree
void ast_print(struct ast_node *root, int depth);

#endif
