#ifndef LFR_AST_H
#define LFR_AST_H

#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/utils/bits.h>

#include <inplocation.h>
#include <ast/arr_decls.h>
#include <ast/type_decls.h>
#include <ast/ast_utils.h>
#include <ast/typeclass_decls.h>
#include <ast/generics_decls.h>
#include <ast/vardecl_decls.h>
#include <ast/function_decls.h>
#include <ast/string_literal_decls.h>
#include <ast/identifier.h>
#include <ast/constants_decls.h>
#include <ast/operators_decls.h>
#include <ast/ifexpr_decls.h>
#include <ast/forexpr_decls.h>
#include <ast/iterable_decls.h>
#include <ast/matchexpr_decls.h>
#include <ast/block_decls.h>
#include <ast/returnstmt_decls.h>
#include <ast/module_decls.h>
#include <types/type_elementary.h>

#include <analyzer/symbol_table.h>

/**
 * Performs an assert on the type of the AST node in debug mode
 */
#define AST_NODE_ASSERT_TYPE(node_, type_)              \
    RF_ASSERT(                                          \
        (node_)->type == type_,                         \
        "Illegal ast node type. Expected \""RFS_PF"\""  \
        " but encountered \""RFS_PF"\"",                \
        RFS_PA(ast_nodetype_str(type_)),                \
        RFS_PA(ast_node_str(node_))                     \
    )

/**
 * Performs an assert on the state of the AST node in debug mode
 */
#define AST_NODE_ASSERT_STATE(node_, state_)                \
    RF_ASSERT(                                              \
        (node_)->state >= state_,                           \
        "Illegal ast node state. Expected \""               \
        "state >= "RFS_PF"\" but encountered \""RFS_PF"\"", \
        RFS_PA(ast_nodestate_str(state_)),                  \
        RFS_PA(ast_nodestate_str((node_)->state))           \
    )

#define AST_PRINT_DEPTHMUL 4

struct ast_root {
    struct symbol_table st;
};

enum ast_type {
    AST_ROOT = 0,
    AST_ARRAY_SPEC,
    AST_BLOCK,
    AST_VARIABLE_DECLARATION,
    AST_RETURN_STATEMENT,
    AST_TYPE_DECLARATION,
    AST_TYPE_OPERATOR,
    AST_TYPE_LEAF,
    AST_TYPE_DESCRIPTION,
    AST_TYPECLASS_DECLARATION,
    AST_TYPECLASS_INSTANCE,
    AST_GENERIC_DECLARATION,
    AST_GENERIC_TYPE,
    AST_GENERIC_ATTRIBUTE,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_IMPLEMENTATION,
    AST_FUNCTION_CALL,
    AST_INDEX_ACCESS,
    AST_CONDITIONAL_BRANCH,
    AST_IF_EXPRESSION,
    AST_FOR_EXPRESSION,
    AST_ITERABLE,
    AST_MATCH_EXPRESSION,
    AST_MATCH_CASE,
    AST_MODULE,
    AST_IMPORT,
    AST_XIDENTIFIER,
    AST_BINARY_OPERATOR,
    AST_UNARY_OPERATOR,
    AST_BRACKET_LIST,

    /* from this value and under all types should have no children */
    AST_STRING_LITERAL,
    AST_IDENTIFIER,
    AST_CONSTANT,
    AST_PLACEHOLDER,

    AST_TYPES_COUNT /* always last */
};

//! States that an ast node goes through during its lifetime
enum ast_node_state {
    AST_NODE_STATE_CREATED = 0,    /*!< State node is in at initial creation */
    AST_NODE_STATE_AFTER_PARSING,  /*!< State after parsing has been succesfull */
    AST_NODE_STATE_ANALYZER_PASS1, /*!< State after the first pass of the analyzer */
    AST_NODE_STATE_TYPECHECK_1, /*!< Context-dependent typechecking state */
    AST_NODE_STATE_RIR_END         /*!< State after the rir types have been created. Last state before backend */
};

struct ast_node {
    enum ast_type type;
    enum ast_node_state state;
    const struct type *expression_type;
    struct inplocation location;
    struct arr_ast_nodes children;
    union {
        struct ast_root root;
        struct ast_block block;
        struct ast_bracketlist bracketlist;
        struct ast_identifier identifier;
        struct ast_xidentifier xidentifier;
        struct ast_vardecl vardecl;
        struct ast_typedecl typedecl;
        struct ast_typeleaf typeleaf;
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
        struct ast_forexpr forexpr;
        struct ast_iterable iterable;
        struct ast_matchcase matchcase;
        struct ast_matchexpr matchexpr;
        struct ast_module module;
        struct ast_import import;
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
struct ast_node *ast_node_create_marks(
    enum ast_type type,
    const struct inplocation_mark *start,
    const struct inplocation_mark *end);
struct ast_node *ast_node_create_ptrs(
    enum ast_type type,
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

void ast_node_add_child(struct ast_node *parent, struct ast_node *child);
void ast_node_copy_children(struct ast_node *parent, struct arr_ast_nodes *children);

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
    if (num > darray_size(n->children) - 1) {
        return NULL;
    }
    return darray_item(n->children, num);
}

i_INLINE_DECL unsigned int ast_node_get_children_number(const struct ast_node *n)
{
    return darray_size(n->children);
}

i_INLINE_DECL const char *ast_node_startsp(const struct ast_node *n)
{
    return n->location.start.p;
}

i_INLINE_DECL const char *ast_node_endsp(const struct ast_node *n)
{
    return n->location.end.p;
}

i_INLINE_DECL const struct inplocation *ast_node_location(const struct ast_node *n)
{
    return &n->location;
}

i_INLINE_DECL const struct inplocation_mark *ast_node_startmark(const struct ast_node *n)
{
    return &n->location.start;
}

i_INLINE_DECL const struct inplocation_mark *ast_node_endmark(const struct ast_node *n)
{
    return &n->location.end;
}

i_INLINE_DECL enum ast_type ast_node_type(const struct ast_node *n)
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

/**
 * Retrieve the type of an ast node
 *
 * @param n         The node whose type to retrieve
 * @return          The type of the node
 */
i_INLINE_DECL const struct type *ast_node_get_type(const struct ast_node *n)
{
    return n->expression_type;
}

i_INLINE_DECL const struct type *ast_node_get_type_or_die(const struct ast_node *n)
{
    const struct type *ret = ast_node_get_type(n);
    RF_ASSERT_OR_EXIT(ret, "ast_node contained NULL type");
    return ret;
}

i_INLINE_DECL const struct type *ast_node_get_type_or_nil(const struct ast_node *n)
{
    return n ? ast_node_get_type(n) : type_elementary_get_type(ELEMENTARY_TYPE_NIL);
}

const struct RFstring *ast_nodetype_str(enum ast_type type);
const struct RFstring *ast_nodestate_str(enum ast_node_state state);
const struct RFstring *ast_node_str(const struct ast_node *n);
struct ast_node *ast_node_placeholder();

struct symbol_table *ast_node_symbol_table_get(struct ast_node *n);


/* -- ast_root functions -- */
struct ast_node *ast_root_create(struct inpfile *file);

i_INLINE_DECL struct symbol_table *ast_root_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_ROOT);
    return &n->root.st;
}


// temporary function, to visualize an ast tree
void ast_print(struct ast_node *root, struct inpfile *f, int depth);
#endif
