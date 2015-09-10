#ifndef LFR_IR_FUNCTION
#define LFR_IR_FUNCTION

#include <RFintrusive_list.h>
#include <Data_Structures/darray.h>
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
    //! Array of all basic blocks under the function
    struct {darray(struct rir_block*);} blocks;
    //! Stringmap from rir identifiers to rir objects
    struct rirobj_strmap map;
    //! Label pointing to the function's end
    struct rir_value *end_label;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx);
void rir_fndecl_destroy(struct rir_fndecl *f);

void rir_fndecl_add_block(struct rir_fndecl *f, struct rir_block *b);
bool rir_fndecl_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *f);

#endif
