#ifndef LFR_IR_FUNCTION
#define LFR_IR_FUNCTION

#include <RFintrusive_list.h>
#include <Data_Structures/darray.h>
#include <ir/rir_strmap.h>

struct ast_node *n;
struct rir_block;
struct rir_ctx;

struct rir_fndecl {
    const struct RFstring *name;
    struct {darray(struct rir_type*);} arguments;
    struct {darray(struct rir_type*);} returns;
    unsigned int args_num;
    struct rir_block *body;
    //! Stringmap for the function's rir object
    struct rirexpr_strmap map;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx);
void rir_fndecl_destroy(struct rir_fndecl *f);

#endif
