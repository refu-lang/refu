#ifndef LFR_IR_FUNCTION
#define LFR_IR_FUNCTION

#include <RFintrusive_list.h>
#include <Data_Structures/darray.h>
#include <Utils/container_of.h>
#include <ir/rir_strmap.h>
#include <ir/rir_argument.h>

struct ast_node *n;
struct rir_block;
struct rir_ctx;
struct rir;

struct rir_fndecl {
    const struct RFstring *name;
    //! Array of the function's arguments
    struct args_arr arguments;
    //! Return type of the function
    struct rir_ltype *return_type;
    //! Differentiate between rir objects that are only with a declaration (true), and those with a body (false)
    bool plain_decl;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};
struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx);
void rir_fndecl_destroy(struct rir_fndecl *f);
bool rir_fndecl_nocheck_tostring(struct rirtostr_ctx *ctx, bool is_plain, const struct rir_fndecl *f);
i_INLINE_DECL bool rir_fndecl_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *f)
{
    return rir_fndecl_nocheck_tostring(ctx, f->plain_decl, f);
}


struct rir_fndef {
    struct rir_fndecl decl;
    //! Array of all basic blocks under the function
    struct {darray(struct rir_block*);} blocks;
    //! Stringmap from rir identifiers to rir objects
    struct rirobj_strmap map;
    //! Label pointing to the function's end
    struct rir_value *end_label;
};


struct rir_fndef *rir_fndef_create(const struct ast_node *n, struct rir_ctx *ctx);
void rir_fndef_destroy(struct rir_fndef *f);

void rir_fndef_add_block(struct rir_fndef *f, struct rir_block *b);
bool rir_fndef_tostring(struct rirtostr_ctx *ctx, const struct rir_fndef *f);
i_INLINE_DECL struct rir_fndef *rir_fndecl_to_fndef(const struct rir_fndecl* d)
{
    return container_of(d, struct rir_fndef, decl);
}

/* -- Generic Methods that will work for both declaration and definitions -- */

bool rir_function_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *fn);
#endif
