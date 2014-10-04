#ifndef LFR_AST_H
#define LFR_AST_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <inplocation.h>
#include <ast/type_decls.h>
#include <ast/typeclass_decls.h>
#include <ast/generics_decls.h>
#include <ast/vardecl.h>
#include <ast/function_decls.h>
#include <ast/string_literal_decls.h>
#include <ast/identifier.h>
#include <ast/constant_num_decls.h>
#include <ast/operators_decls.h>
#include <ast/arrayref_decls.h>

#define AST_PRINT_DEPTHMUL 4

enum ast_type {
    AST_ROOT = 0,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,
    AST_TYPE_DECLARATION,
    AST_TYPE_OPERATOR,
    AST_TYPE_DESCRIPTION,
    AST_TYPECLASS_DECLARATION,
    AST_GENERIC_DECLARATION,
    AST_GENERIC_TYPE,
    AST_GENERIC_ATTRIBUTE,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_CALL,
    AST_ARRAY_REFERENCE,
    AST_XIDENTIFIER,
    AST_BINARY_OPERATOR,
    AST_UNARY_OPERATOR,

    /* from this value and under all types should have no children */
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_CONSTANT_NUMBER,

    AST_TYPES_COUNT /* always last */
};

struct ast_node {
    enum ast_type type;
    struct inplocation location;
    struct RFilist_node lh;
    struct RFilist_head children;
    union {
        struct ast_identifier identifier;
        struct ast_xidentifier xidentifier;
        struct ast_vardecl vardecl;
        struct ast_typedecl typedecl;
        struct ast_typeop typeop;
        struct ast_typedesc typedesc;
        struct ast_typeclass typeclass;
        struct ast_genrtype genrtype;
        struct ast_fndecl fndecl;
        struct ast_fncall fncall;
        struct ast_arrayref arrayref;
        struct ast_binaryop binaryop;
        struct ast_unaryop unaryop;
        struct ast_string_literal string_literal;
        struct ast_constantnum constantnum;
    };
};

void ast_node_init(struct ast_node * n, enum ast_type type);
struct ast_node *ast_node_create(enum ast_type type);

struct ast_node *ast_node_create_loc(enum ast_type type,
                                     struct inplocation *loc);
struct ast_node *ast_node_create_marks(enum ast_type type,
                                       struct inplocation_mark *start,
                                       struct inplocation_mark *end);
struct ast_node *ast_node_create_ptrs(enum ast_type type,
                                      struct inpfile *f,
                                      char *sp, char *ep);

void ast_node_destroy(struct ast_node *n);

void ast_node_set_start(struct ast_node *n, struct inplocation_mark *start);
void ast_node_set_end(struct ast_node *n, struct inplocation_mark *end);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);

/*
 * Registers a child if not NULL with a specific type
 * of an ast_node
 */
#define ast_node_register_child(parent_, child_, position_) \
    do {                                                    \
        if (child_) {                                       \
            ast_node_add_child(parent_, child_);            \
        }                                                   \
        (parent_)->position_ = child_;                      \
    } while (0)


i_INLINE_DECL char *ast_node_startsp(struct ast_node *n)
{
    return n->location.start.p;
}

i_INLINE_DECL char *ast_node_endsp(struct ast_node *n)
{
    return n->location.end.p;
}
i_INLINE_DECL struct inplocation_mark *ast_node_startmark(struct ast_node *n)
{
    return &n->location.start;
}
i_INLINE_DECL struct inplocation_mark *ast_node_endmark(struct ast_node *n)
{
    return &n->location.end;
}

const struct RFstring *ast_node_str(struct ast_node *n);

// temporary function, to visualize an ast tree
void ast_print(struct ast_node *root, struct inpfile *f, int depth);
#endif
