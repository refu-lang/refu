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
    RIR_EXPRESSION_LABEL,
    RIR_EXPRESSION_FNCALL,
    RIR_EXPRESSION_ALLOCA,
    RIR_EXPRESSION_RETURN,
    RIR_EXPRESSION_CONSTRUCT,
    RIR_EXPRESSION_WRITE,
    RIR_EXPRESSION_READ,
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

struct rir_label {
    //! The basic block in which the label will be found
    const struct rir_block *block;
    //! The position where the label will be found in the block
    unsigned index;
};

struct rir_expression *rir_alloca_create(const struct rir_ltype *type,
                                         uint64_t num,
                                         struct rir_ctx *ctx);
struct rir_expression *rir_return_create(const struct rir_expression *val, struct rir_ctx *ctx);
bool rir_return_init(struct rir_expression *ret,
                     const struct rir_expression *val,
                     struct rir_ctx *ctx);
struct rir_expression *rir_constant_create(const struct ast_node *c, struct rir_ctx *ctx);
struct rir_expression *rir_label_create(const struct rir_block *b, unsigned index, struct rir_ctx *ctx);
struct rir_expression *rir_label_string_create(const struct rir_block *b, const struct RFstring *str, unsigned index, struct rir_ctx *ctx);

struct rir_expression {
    enum rir_expression_type type;
    union {
        struct rir_fncall fncall;
        struct rir_alloca alloca;
        struct rir_binaryop binaryop;
        struct rir_return ret;
        struct rir_label label;
        // kind of ugly but it's exactly what we need
        struct ast_constant constant;
    };
    struct rir_value val;
    // Control to be added to expression list of a rir block
    struct RFilist_node ln;
};

bool rir_expression_init(struct rir_expression *expr,
                         enum rir_expression_type type,
                         struct rir_ctx *ctx);
void rir_expression_destroy(struct rir_expression *expr);
bool rir_expression_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e);
#endif
