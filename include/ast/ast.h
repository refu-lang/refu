#ifndef LFR_AST_H
#define LFR_AST_H

#include <RFintrusive_list.h>
#include <RFstring.h>

#include <inplocation.h>
#include <ast/type_decls.h>
#include <ast/typeclass_decls.h>
#include <ast/generics_decls.h>
#include <ast/vardecl_decls.h>
#include <ast/function_decls.h>
#include <ast/string_literal_decls.h>
#include <ast/identifier.h>
#include <ast/constants_decls.h>
#include <ast/operators_decls.h>
#include <ast/ifexpr_decls.h>
#include <ast/block_decls.h>
#include <ast/returnstmt_decls.h>

#include <analyzer/symbol_table.h>

/**
 * Performs an assert on the type of the AST node in debug mode
 */
#define AST_NODE_ASSERT_TYPE(node_, type_)                              \
    RF_ASSERT((node_)->type == type_, "Illegal ast node type. Expected \"" \
              RF_STR_PF_FMT"\" but encountered \""RF_STR_PF_FMT"\"",    \
              RF_STR_PF_ARG(ast_nodetype_str(type_)),                   \
              RF_STR_PF_ARG(ast_node_str(node_)))

#define AST_PRINT_DEPTHMUL 4

struct ast_root {
    struct symbol_table st;
};

enum ast_type {
    AST_ROOT = 0,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,
    AST_RETURN_STATEMENT,
    AST_TYPE_DECLARATION,
    AST_TYPE_OPERATOR,
    AST_TYPE_DESCRIPTION,
    AST_TYPECLASS_DECLARATION,
    AST_TYPECLASS_INSTANCE,
    AST_GENERIC_DECLARATION,
    AST_GENERIC_TYPE,
    AST_GENERIC_ATTRIBUTE,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_IMPLEMENTATION,
    AST_FUNCTION_CALL,
    AST_ARRAY_REFERENCE,
    AST_CONDITIONAL_BRANCH,
    AST_IF_EXPRESSION,
    AST_XIDENTIFIER,
    AST_BINARY_OPERATOR,
    AST_UNARY_OPERATOR,

    /* from this value and under all types should have no children */
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_CONSTANT,

    AST_TYPES_COUNT /* always last */
};

//! States that an ast node goes through during its lifetime
enum ast_node_state {
    AST_NODE_STATE_CREATED = 0,   /*!< State node is in at initial creation */
    AST_NODE_STATE_AFTER_PARSING, /*!< State after parsing has been succesfull */
    AST_NODE_STATE_ANALYZER_PASS1 /*!< State after the first pass of the analyzer */
};

struct ast_node {
    enum ast_type type;
    enum ast_node_state state;
    const struct type *expression_type;
    struct inplocation location;
    struct RFilist_node lh;
    struct RFilist_head children;
    union {
        struct ast_root root;
        struct ast_block block;
        struct ast_identifier identifier;
        struct ast_xidentifier xidentifier;
        struct ast_vardecl vardecl;
        struct ast_typedecl typedecl;
        struct ast_typeop typeop;
        struct ast_typedesc typedesc;
        struct ast_typeclass typeclass;
        struct ast_typeinstance typeinstance;
        struct ast_genrtype genrtype;
        struct ast_fndecl fndecl;
        struct ast_fnimpl fnimpl;
        struct ast_fncall fncall;
        struct ast_condbranch condbranch;
        struct ast_ifexpr ifexpr;
        struct ast_binaryop binaryop;
        struct ast_unaryop unaryop;
        struct ast_string_literal string_literal;
        struct ast_constant constant;
        struct ast_returnstmt returnstmt;
    };
};

void ast_node_init(struct ast_node *n, enum ast_type type);
struct ast_node *ast_node_create(enum ast_type type);

struct ast_node *ast_node_create_loc(enum ast_type type,
                                     const struct inplocation *loc);
struct ast_node *ast_node_create_marks(enum ast_type type,
                                       const struct inplocation_mark *start,
                                       const struct inplocation_mark *end);
struct ast_node *ast_node_create_ptrs(enum ast_type type,
                                      struct inpfile *f,
                                      char *sp, char *ep);

void ast_node_destroy(struct ast_node *n);
/**
 * Destroy node if it's a token generated ast node and needs
 * to be destroyed during lexer destruction.
 */
void ast_node_destroy_from_lexer(struct ast_node *n);

void ast_node_set_start(struct ast_node *n, const struct inplocation_mark *start);
void ast_node_set_end(struct ast_node *n, const struct inplocation_mark *end);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);

/**
 * Depending on the node type, it finds the child node
 * that identifies it and returns the string of the identifier
 *
 * @param n             The node in question
 * @return              Either the string of the identifying node or NULL
 *                      if not applicable
 */
const struct RFstring * ast_node_get_name_str(const struct ast_node *n);

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

i_INLINE_DECL struct ast_node *ast_node_get_child(struct ast_node *n,
                                                  unsigned int num)
{
    struct ast_node *child;
    rf_ilist_for_each(&n->children, child, lh) {
        if (num == 0) {
            return child;
        }
        num --;
    }

    return NULL;
}

i_INLINE_DECL char *ast_node_startsp(struct ast_node *n)
{
    return n->location.start.p;
}

i_INLINE_DECL char *ast_node_endsp(struct ast_node *n)
{
    return n->location.end.p;
}

i_INLINE_DECL struct inplocation *ast_node_location(struct ast_node *n)
{
    return &n->location;
}

i_INLINE_DECL struct inplocation_mark *ast_node_startmark(struct ast_node *n)
{
    return &n->location.start;
}

i_INLINE_DECL struct inplocation_mark *ast_node_endmark(struct ast_node *n)
{
    return &n->location.end;
}

i_INLINE_DECL enum ast_type ast_node_type(struct ast_node *n)
{
    return n->type;
}

/**
 * Returns true if the node has a value associated with it.
 * Is used in the parser-lexer connection to deal with memory ownership.
 * 
 * The constant boolean is an exception of a node that DOES NOT return true here
 * since the lexer does not create it
 */
i_INLINE_DECL bool ast_node_has_value(const struct ast_node *n)
{
    return n->type == AST_IDENTIFIER ||
        n->type == AST_STRING_LITERAL ||
        (n->type == AST_CONSTANT && n->constant.type != CONSTANT_BOOLEAN);
}

i_INLINE_DECL const struct type *ast_expression_get_type(struct ast_node *expr)
{
    return expr->expression_type;
}

const struct RFstring *ast_nodetype_str(enum ast_type type);
const struct RFstring *ast_node_str(const struct ast_node *n);

struct symbol_table *ast_node_symbol_table_get(struct ast_node *n);


/* -- ast_root functions -- */
struct ast_node *ast_root_create(struct inpfile *file);

i_INLINE_DECL bool ast_root_symbol_table_init(struct ast_node *n,
                                              struct analyzer *a)
{
    AST_NODE_ASSERT_TYPE(n, AST_ROOT);
    return symbol_table_init(&n->root.st, a);
}

i_INLINE_DECL struct symbol_table *ast_root_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_ROOT);
    return &n->root.st;
}


// temporary function, to visualize an ast tree
void ast_print(struct ast_node *root, struct inpfile *f, int depth);
#endif
