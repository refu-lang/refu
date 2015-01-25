#ifndef LFR_IR_ELEMENTS_H
#define LFR_IR_ELEMENTS_H

#include <Data_Structures/darray.h>
#include <Utils/struct_utils.h>
#include <types/type_decls.h>

struct type;
struct RFstring;
struct ast_node;
struct symbol_table;

/**
 * Representation of a type for the Refu IR
 *
 * It is much like @see struct type but with some constraints.
 * It is represented by an array of subtypes and not by a tree. A single
 * rir_type can only contain subtypes that are connected by the same type
 * operation.
 *
 * TODO: In order to achieve this all possible composite types need to be defined
 * somewhere. Figure out where and do it.
 */
struct rir_type {
    //! TODO: This should be an enum with all elementary types + 1 for composite type
    //! In the case of composite type just define it somewhere else so that backends
    //! like LLVM can have it as a struct
    enum elementary_type elementary;
    //! Array of types that may constitute this type. e.g: i64, u32, string
    struct {darray(struct rir_type*);} subtypes;
    //! Whether the @c subtypes are connected as product types (so by ,) or
    //! as sum types (so by | )  TODO: Think if we will use this
    bool are_product_type;
    //! Name of the variable the type describes. TODO: Maybe move this somwhere
    //! else. Separate the notion of type from parameter?
    struct RFstring name;
};
struct rir_type *rir_type_alloc(const struct type *input);
struct rir_type *rir_type_create(const struct type *input,
                                 const struct RFstring *name);
bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name);

void rir_type_dealloc(struct rir_type *t);
void rir_type_destroy(struct rir_type *t);
void rir_type_deinit(struct rir_type *t);

//! @return the name of the nth parameter of the type
const struct RFstring *rir_type_get_nth_name(struct rir_type *t, unsigned n);
//! @return the type of the nth parameter of the type
const struct rir_type *rir_type_get_nth_type(struct rir_type *t, unsigned n);

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
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_function, struct ast_node *fn_impl);


struct rir_simple_branch {
    struct rir_basic_block *dst;
};

struct rir_cond_branch {
    struct rir_simple_branch true_br;
    struct rir_simple_branch false_br;
    struct ast_expression *cond;
};

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
        struct rir_simple_branch simple_branch;
    };
};

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
    struct RFilist_head lh;
    //! exit branch
    struct rir_branch exit;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_basic_block);
struct rir_basic_block *rir_basic_blocks_create_from_ast_block(struct ast_node *n);

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
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_module, struct ast_node *n);

#endif
