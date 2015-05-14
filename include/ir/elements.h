#ifndef LFR_IR_ELEMENTS_H
#define LFR_IR_ELEMENTS_H

#include <Data_Structures/darray.h>
#include <Utils/struct_utils.h>
#include <Data_Structures/intrusive_list.h>

struct RFstring;
struct ast_node;
struct symbol_table;
struct rir;
struct rir_basic_block;
struct rir_branch;

/**
 * Representation of a function for the Refu IR
 */
struct rir_function {
    //! Function symbols
    struct symbol_table *symbols;
    struct rir_type *arg_type;
    struct rir_type *ret_type;
    struct rir_basic_block *entry;
    struct RFstring name;

    /*-- control for module list --*/
    struct RFilist_node ln_for_module;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_function, struct ast_node *fn_impl, struct rir *rir);

struct rir_cond_branch {
    struct rir_basic_block *true_br;
    struct rir_branch *false_br;
    struct ast_node *cond;
};

bool rir_cond_branch_init(struct rir_cond_branch *rir_cond,
                          struct ast_node *n,
                          struct symbol_table *st,
                          struct rir *rir);
struct rir_cond_branch *rir_cond_branch_create(struct ast_node *n,
                                               struct symbol_table *st,
                                               struct rir *rir);

void rir_cond_branch_deinit(struct rir_cond_branch *rir_cond);

/**
 * Represents a branching point in the Refu IR.
 *
 * A branching can either be conditional or unconditional in which
 * case only a single destination needs to be specified.
 */
struct rir_branch {
    bool is_conditional;
    union {
        struct rir_cond_branch cond_branch;
        struct rir_basic_block *simple_branch;
    };
};

struct rir_branch *rir_branch_create(struct ast_node *node,
                                     struct symbol_table *st,
                                     struct rir *rir);
void rir_branch_destroy(struct rir_branch *branch);

/**
 * Represents a basic block in the Refu IR.
 *
 * A basic block can only have some uninterrupted expressions and statements
 * and has only one single exit.
 */
struct rir_basic_block {
    //! Block symbols
    struct symbol_table *symbols;
    //! List of expressions
    struct RFilist_head expressions;
    //! exit branch
    struct rir_branch exit;
    //! Is a normal block or not
    bool normal_block;
    //! [optional] condition branch. If this is not NULL then this is
    //! an else if block and this will be its rir condition
    struct rir_branch *condition;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_basic_block);
struct rir_basic_block *rir_basic_blocks_create_from_ast_block(
    struct ast_node *n,
    struct symbol_table *st,
    struct rir *rir);

bool rir_handle_block_expression(struct ast_node *n, struct rir_basic_block *b, struct rir *rir);

/**
 * Represents an expressions in the RIR
 */
struct rir_expression {
    enum rir_expression_type {
        RIR_SIMPLE_EXPRESSION,
        RIR_IF_EXPRESSION
    } type;

    union {
        //! Pointer to the ast node
        struct ast_node *expr;
        struct rir_branch *branch;
    };
    //! Handler to be added to the block
    struct RFilist_node ln;
};

bool rir_expression_init(struct rir_expression *expr,
                         struct rir_basic_block *parent,
                         struct ast_node *node,
                         enum rir_expression_type type,
                         struct rir *rir);

struct rir_expression *rir_expression_create(struct rir_basic_block *parent,
                                             struct ast_node *node,
                                             enum rir_expression_type type,
                                             struct rir *rir);

void rir_expression_deinit(struct rir_expression *expr);
void rir_expression_destroy(struct rir_expression *expr);

/**
 * Represents a module in the IR
 *
 * It's basically (for now) a set of functions and one main function
 */
struct rir_module {
    //! Global symbols of the module
    struct symbol_table *symbols;
    //! List of functions of the module
    struct RFilist_head functions;
    //! Name of a module
    struct RFstring name;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_module, struct ast_node *n,
                               const struct RFstring *name,
                               struct rir *rir);

#endif
