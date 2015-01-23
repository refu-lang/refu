#ifndef LFR_IR_ELEMENTS_H
#define LFR_IR_ELEMENTS_H

#include <Data_Structures/darray.h>
#include <Utils/struct_utils.h>
#include <types/type_decls.h>

struct type;
struct RFstring;
struct ast_expression;

/**
 * Representation of a type for the Refu IR
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

    /* -- control attributes -- */
    //! To be added to parameter lists (?)
    //! TODO: Think if we will use this
    struct RFilist_node lh;
};
struct rir_type *rir_type_alloc(struct type *input);
struct rir_type *rir_type_create(struct type *input);
bool rir_type_init(struct rir_type *type, struct type *input);

void rir_type_dealloc(struct rir_type *t);
void rir_type_destroy(struct rir_type *t);
void rir_type_deinit(struct rir_type *t);

/**
 * Representation of a function for the Refu IR
 */
struct rir_function {
    struct rir_type *arg_type;
    struct rir_type *ret_type;
    struct rir_basic_block *entry;
    struct RFstring name;
};
RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_function, struct type *fn_type,
                               struct RFstring *name);


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
    //! List of expressions (?)

    //! exit branch
    struct rir_branch exit;
};

RF_STRUCT_COMMON_SIGS_NO_ALLOC(rir_basic_block, int a);


#endif
