#ifndef LFR_IR_RIR_EXPRESSION_H
#define LFR_IR_RIR_EXPRESSION_H

#include <RFintrusive_list.h>
#include <stdint.h>
#include <ast/constants_decls.h>
#include <ir/rir_value.h>
#include <ir/rir_argument.h>

struct ast_node;
struct rir;
struct rir_ctx;
struct rirtostr_ctx;

enum rir_expression_type {
    RIR_EXPRESSION_CALL,
    RIR_EXPRESSION_ALLOCA,
    RIR_EXPRESSION_RETURN,
    RIR_EXPRESSION_CONVERT,
    RIR_EXPRESSION_WRITE,
    RIR_EXPRESSION_READ,
    RIR_EXPRESSION_OBJMEMBERAT,
    RIR_EXPRESSION_SETUNIONIDX,
    RIR_EXPRESSION_GETUNIONIDX,
    RIR_EXPRESSION_UNIONMEMBERAT,
    RIR_EXPRESSION_CONSTANT,
    RIR_EXPRESSION_ADD,
    RIR_EXPRESSION_SUB,
    RIR_EXPRESSION_MUL,
    RIR_EXPRESSION_DIV,
    RIR_EXPRESSION_CMP_EQ,
    RIR_EXPRESSION_CMP_NE,
    RIR_EXPRESSION_CMP_GE,
    RIR_EXPRESSION_CMP_GT,
    RIR_EXPRESSION_CMP_LE,
    RIR_EXPRESSION_CMP_LT,
    RIR_EXPRESSION_LOGIC_AND,
    RIR_EXPRESSION_LOGIC_OR,
    // PLACEHOLDER, should not make it into any expression type
    RIR_EXPRESSION_PLACEHOLDER
};


/**
 *  RIR call specific members of an expression.
 *  @note: The return value which is also the value of the function call
 *  is part of the containing expression and not of this struct
 */
struct rir_call {
    //! Name of the function call
    struct RFstring name;
    //! An array of values that comprise this call's arguments
    struct value_arr args;
    //! True if this is a call to a foreign function
    bool foreign;
};

enum alloc_location {
    RIR_ALLOC_STACK,
    RIR_ALLOC_HEAP
};

struct rir_alloca {
    //! Type to allocate
    struct rir_type *type;
    //! Where to allocate the rir object
    enum alloc_location alloc_location;
    //! Id of the corresponding allocation in the actual code. If this alloca
    //! has no corresponding ast id then this is NULL
    const struct RFstring *ast_id;
};

struct rir_return {
    //! The value to return or NULL if it's a return without expression
    const struct rir_value *val;
};

struct rir_binaryop {
    const struct rir_value *a;
    const struct rir_value *b;
};

/**
 * Reads the contents of a memory area (pointer)
 */
struct rir_read {
    //! Memory value to read from
    const struct rir_value *memory;
};

/**
 * Write to a memory area
 */
struct rir_write {
    //! Memory value to write to
    const struct rir_value *memory;
    //! Value to write into that area
    const struct rir_value *writeval;
};

/**
 * Converts a value from one type to another
 */
struct rir_convert {
    //! Value to convert
    const struct rir_value *val;
    //! Type to convert the value to
    const struct rir_type *type;
};

/**
 * Returns a pointer to an object's member at a given index
 */
struct rir_objmemberat {
    //! The object's memory
    const struct rir_value *objmemory;
    //! The index of the member to retrieve
    uint32_t idx;
};

/**
 * Just like @ref rir_objmemberat but for unions
 */
struct rir_unionmemberat {
    //! The unions's memory
    const struct rir_value *unimemory;
    //! The index of the member to retrieve
    uint32_t idx;
};

struct rir_setunionidx {
    const struct rir_value *unimemory;
    const struct rir_value *idx;
};

struct rir_getunionidx {
    const struct rir_value *unimemory;
};


struct rir_object *rir_alloca_create_obj(
    struct rir_type *type,
    const struct RFstring *id,
    enum rir_pos pos,
    rir_data data
);
struct rir_expression *rir_alloca_create(
    struct rir_type *type,
    const struct RFstring *id,
    enum rir_pos pos,
    rir_data data
);

struct rir_expression *rir_setunionidx_create(
    const struct rir_value *unimemory,
    const struct rir_value *idx,
    enum rir_pos pos,
    rir_data data
);
struct rir_expression *rir_getunionidx_create(
    const struct rir_value *unimemory,
    enum rir_pos pos,
    rir_data data
);

struct rir_expression *rir_unionmemberat_create(
    const struct rir_value *unimemory,
    uint32_t idx,
    enum rir_pos pos,
    rir_data data
);

struct rir_object *rir_objmemberat_create_obj(
    const struct rir_value *objmemory,
    uint32_t idx,
    enum rir_pos pos,
    rir_data data
);
struct rir_expression *rir_objmemberat_create(
    const struct rir_value *objmemory,
    uint32_t idx,
    enum rir_pos pos,
    rir_data data
);

struct rir_expression *rir_read_create(
    const struct rir_value *memory_to_read,
    enum rir_pos pos,
    rir_data data
);
struct rir_object *rir_read_create_obj(
    const struct rir_value *memory_to_read,
    enum rir_pos pos,
    rir_data data
);

struct rir_expression *rir_write_create(
    const struct rir_value *memory_to_write,
    const struct rir_value *writeval,
    enum rir_pos pos,
    rir_data data
);
struct rir_object *rir_write_create_obj(
    const struct rir_value *memory_to_write,
    const struct rir_value *writeval,
    enum rir_pos pos,
    rir_data data
);

struct rir_object *rir_return_create(const struct rir_expression *val, struct rir_ctx *ctx);
void rir_return_init(struct rir_return *ret, const struct rir_value *val);

struct rir_expression {
    enum rir_expression_type type;
    union {
        struct rir_convert convert;
        struct rir_call call;
        struct rir_alloca alloca;
        struct rir_setunionidx setunionidx;
        struct rir_getunionidx getunionidx;
        struct rir_unionmemberat unionmemberat;
        struct rir_objmemberat objmemberat;
        struct rir_binaryop binaryop;
        struct rir_read read;
        struct rir_write write;
    };
    struct rir_value val;
    // Control to be added to expression list of a rir block
    struct RFilist_node ln;
};

/**
 * Initialize a rir  expression object
 *
 * @see rir_value_variable_init()
 */
bool rir_object_expression_init(
    struct rir_object *obj,
    enum rir_expression_type type,
    enum rir_pos pos,
    rir_data data
);

void rir_expression_deinit(struct rir_expression *expr);
bool rir_expression_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);
const struct RFstring *rir_expression_type_string(const struct rir_expression *expr);
struct rir_object *rir_expression_to_obj(struct rir_expression *expr);
#endif
