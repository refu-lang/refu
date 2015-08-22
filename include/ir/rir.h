#ifndef LFR_IR_RIR
#define LFR_IR_RIR

#include <RFintrusive_list.h>

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

struct rir_ctx {
    struct rir *rir;
    struct rir_fndecl *current_fn;
    struct rir_block *current_block;
    unsigned expression_idx;
};

void rirctx_block_add(struct rir_ctx *ctx, struct rir_expression *expr);
#endif
