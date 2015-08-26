#ifndef LFR_IR_RIR
#define LFR_IR_RIR

#include <RFintrusive_list.h>
#include <Data_Structures/darray.h>

struct module;
struct compiler;
struct rir_types_list;
struct rir_expression;
struct RFstringx;

struct rir {
    //! A list of all rir types of the file
    struct rir_types_list *rir_types_list;
    //! List of functions
    struct RFilist_head functions;
    //! List of type definitions
    struct RFilist_head typedefs;
    //! Buffer string to hold the string representation when asked. Can be NULL.
    struct RFstringx *buff;
};

struct RFstring *rir_tostring(struct rir *r);

struct rir *rir_create(struct module *m);
void rir_destroy(struct rir* r);

bool rir_process(struct compiler *c);
bool rir_print(struct compiler *c);

struct rir_typedef *rir_typedef_byname(const struct rir *r, const struct RFstring *name);
struct rir_ltype *rir_type_byname(const struct rir *r, const struct RFstring *name);


struct rir_ctx {
    struct rir *rir;
    struct rir_fndecl *current_fn;
    struct rir_block *current_block;
    struct rir_expression *returned_expr;
    unsigned expression_idx;
};

#define RIRCTX_RETURN_EXPR(i_ctx_, i_result_, i_expression_)    \
    do {                                                        \
        (i_ctx_)->returned_expr = i_expression_;                \
        return i_result_;                                       \
    } while (0)

void rirctx_block_add(struct rir_ctx *ctx, struct rir_expression *expr);


struct rirtostr_ctx {
    struct rir *rir;
    struct {darray(const struct rir_block*);} visited_blocks;
};
void rirtostr_ctx_init(struct rirtostr_ctx *ctx, struct rir *r);
void rirtostr_ctx_reset(struct rirtostr_ctx *ctx);
void rirtostr_ctx_visit_block(struct rirtostr_ctx *ctx, const struct rir_block *b);
bool rirtostr_ctx_block_visited(struct rirtostr_ctx *ctx, const struct rir_block *b);
#endif
