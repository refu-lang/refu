#ifndef LFR_IR_RIR_EXPRESSION_H
#define LFR_IR_RIR_EXPRESSION_H

#include <RFintrusive_list.h>
#include <stdint.h>
#include <ast/constants_decls.h>
#include <ir/rir_value.h>

struct ast_node;
struct rir;
struct rir_ctx;
struct rirtostr_ctx;
struct rir_ltype;

enum rir_expression_type {
    RIR_EXPRESSION_FNCALL,
    RIR_EXPRESSION_ALLOCA,
    RIR_EXPRESSION_RETURN,
    RIR_EXPRESSION_CONSTRUCT,
    RIR_EXPRESSION_WRITE,
    RIR_EXPRESSION_READ,
    RIR_EXPRESSION_OBJMEMBERAT,
    RIR_EXPRESSION_SETUNIONIDX,
    RIR_EXPRESSION_UNIONMEMBERAT,
    RIR_EXPRESSION_CONSTANT,
    RIR_EXPRESSION_ADD,
    RIR_EXPRESSION_SUB,
    RIR_EXPRESSION_MUL,
    RIR_EXPRESSION_DIV,
    RIR_EXPRESSION_CMP,
    RIR_EXPRESSION_LOGIC_AND,
    RIR_EXPRESSION_LOGIC_OR,
    // PLACEHOLDER, should not make it into actual production
    RIR_EXPRESSION_PLACEHOLDER
};


struct rir_fncall {
    const struct RFstring *name;
    // TODO: define the fncall members
};

struct rir_alloca {
    const struct rir_ltype *type;
    uint64_t num;
};

struct rir_return {
    const struct rir_expression *val;
};

struct rir_binaryop {
    const struct rir_value *a;
    const struct rir_value *b;
};

struct rir_read {
    //! Memory value to read from
    const struct rir_value *memory;
};

struct rir_objmemberat {
    const struct rir_value *objmemory;
    uint32_t idx;
};

struct rir_setunionidx {
    const struct rir_value *unimemory;
    uint32_t idx;
};

struct rir_unionmemberat {
    const struct rir_value *unimemory;
    uint32_t idx;
};

struct rir_object *rir_alloca_create_obj(const struct rir_ltype *type,
                                         uint64_t num,
                                         struct rir_ctx *ctx);
struct rir_expression *rir_setunionidx_create(const struct rir_value *unimemory,
                                              uint32_t idx,
                                              struct rir_ctx *ctx);
struct rir_expression *rir_unionmemberat_create(const struct rir_value *unimemory,
                                                uint32_t idx,
                                                struct rir_ctx *ctx);
struct rir_expression *rir_objmemberat_create(const struct rir_value *objmemory,
                                              uint32_t idx,
                                              struct rir_ctx *ctx);
struct rir_expression *rir_read_create(const struct rir_value *memory_to_read,
                                       struct rir_ctx *ctx);
struct rir_object *rir_return_create(const struct rir_expression *val, struct rir_ctx *ctx);
void rir_return_init(struct rir_expression *ret,
                     const struct rir_expression *val);

struct rir_expression {
    enum rir_expression_type type;
    union {
        struct rir_fncall fncall;
        struct rir_alloca alloca;
        struct rir_setunionidx setunionidx;
        struct rir_unionmemberat unionmemberat;
        struct rir_objmemberat objmemberat;
        struct rir_binaryop binaryop;
        struct rir_read read;
        struct rir_return ret;
        // kind of ugly but it's exactly what we need
        struct ast_constant constant;
    };
    struct rir_value val;
    // Control to be added to expression list of a rir block
    struct RFilist_node ln;
};

bool rir_expression_init(struct rir_object *expr,
                         enum rir_expression_type type,
                         struct rir_ctx *ctx);
void rir_expression_destroy(struct rir_expression *expr);
bool rir_expression_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);
#endif
