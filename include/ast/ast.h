#ifndef LFR_AST_H
#define LFR_AST_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <ast/location.h>
#include <ast/identifier.h>
#include <ast/typedecl.h>
#include <ast/typedesc.h>
#include <ast/genrdecl.h>
#include <ast/vardecl.h>
#include <ast/fndecl.h>

#define AST_PRINT_DEPTHMUL 4

enum ast_type {
    AST_ROOT = 0,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,
    AST_TYPE_DECLARATION,
    AST_TYPE_OPERATOR,
    AST_TYPE_DESCRIPTION,
    AST_GENERIC_DECLARATION,
    AST_GENERIC_TYPE,
    AST_FUNCTION_DECLARATION,

    /* from this value and under all types should have no children */
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_TYPES_COUNT /* always last */
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    struct RFilist_head children;
    union {
        struct RFstring identifier;
        struct ast_vardecl vardecl;
        struct ast_typedecl typedecl;
        struct ast_typeop typeop;
        struct ast_genrdecl genrdecl;
        struct ast_genrtype genrtype;
        struct ast_fndecl fndecl;
    };
};

struct ast_node *ast_node_create(enum ast_type type,
                                 struct parser_file *f,
                                 char *sp, char *ep);

void ast_node_destroy(struct ast_node *n);

bool ast_node_set_end(struct ast_node *n, char *end);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);


i_INLINE_DECL char *ast_node_startsp(struct ast_node *n)
{
    return n->location.sp;
}

i_INLINE_DECL char *ast_node_endsp(struct ast_node *n)
{
    return n->location.ep;
}

const struct RFstring *ast_node_str(struct ast_node *n);

// temporary function, to visualize an ast tree
void ast_print(struct ast_node *root, int depth, const char *description);
#endif
