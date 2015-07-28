#include <ir/rir_strmap.h>
#include <ir/rir.h>
#include <ir/rir_block.h>
#include <ir/rir_function.h>
#include <analyzer/symbol_table.h>

#include <String/rf_str_core.h>

/* bool rirexpr_strmap_add_from_id(struct rirexpr_strmap *m, */
/*                                 const struct RFstring *id, */
/*                                 struct rir_expression *e) */
/* { */
/*     // create new val */
/*     val = CREATE(); */
/*     return strmap_add(m, rec->id, VAL); */
/* } */


bool rir_strmaps_add_from_id(struct rir_ctx *ctx,
                             const struct RFstring *id,
                             struct rir_expression *e)
{
    // add normal language identifier to current block
    if (!strmap_add(&ctx->current_block->map, id, e)) {
        return false;
    }
    struct RFstring *uid = rf_string_create_int(ctx->current_fn->symbols_num);
    if (!uid) {
        return false;
    }
    // add unique identifier to function
    RF_ASSERT_OR_EXIT(strmap_add(&ctx->current_fn->map, uid, e), "identifier should have been unique");
    ctx->current_fn->symbols_num++;
    return true;
}
