#ifndef LFR_AST_H
#define LFR_AST_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <ast/location.h>

enum ast_type {
    AST_ROOT = 0,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,
    AST_DATA_DECLARATION,

    /* from this value and under all types should have no children */
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_TYPES_COUNT /* always last */
};


struct ast_vardecl {
    //! identifier of the name
    struct ast_node *name;
    //! identifier of the type
    struct ast_node *type;
};

struct ast_datadecl {
    //! identifier of the name
    struct ast_node *name;
    //! List of ast nodes that are members of the declaration
    struct RFilist_head members;
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    union {
        struct RFstring identifier;
        struct ast_vardecl vardecl;
        struct ast_datadecl datadecl;

        struct RFilist_head children;
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

void ast_vardecl_init(struct ast_node *n,
                      struct ast_node *name,
                      struct ast_node *type);
void ast_datadecl_init(struct ast_node *n, struct ast_node *name);
void ast_datadecl_add_child(struct ast_node *n, struct ast_node *c);
#endif
