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
    const struct rir_type* arguments;
    const struct rir_type* returns;
    struct args_arr arguments_list;
    struct rir_block *body;
    //! Stringmap from rir identifiers to rir objects
    //! Owns the rir objects and the rir identifier.
    //! They will be destroyed at function destruction
    struct rirexpr_strmap map;
    struct rirexpr_strmap id_map;
    //! Label pointing to the function's end
    struct rir_expression *end_label;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx);
void rir_fndecl_destroy(struct rir_fndecl *f);

bool rir_fndecl_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *f);

#endif
