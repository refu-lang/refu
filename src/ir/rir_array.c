#include <ir/rir_array.h>

#include <rfbase/string/manipulationx.h>

#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_process.h>
#include <ir/rir_convert.h>
#include <ir/rir_constant.h>

#include <ast/ast.h>
#include <ast/arr.h>

struct rir_object *rir_fixedarr_create_obj_from_ast(
    const struct ast_node *n,
    struct rir_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BRACKET_LIST);
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, rir_ctx_rir(ctx));
    if (!ret) {
        return NULL;
    }

    struct arr_ast_nodes *astnodes = ast_bracketlist_members((struct ast_node *)n);
    RF_ASSERT(
        darray_size(*astnodes) != 0,
        "Can't RIR process empty bracketlists at the moment"
    );
    struct rir_type *member_type = rir_type_create_from_type(
        ast_node_get_type_or_die(darray_item(*astnodes, 0)),
        RIR_LOC_FIXEDARR,
        ctx
    );
    // initialize fixedarray members
    ret->expr.fixedarr.member_type = member_type;
    ret->expr.fixedarr.size = darray_size(*astnodes);
    // initialize fixed array rir values
    darray_init(ret->expr.fixedarr.members);

    struct ast_node **astmember;
    darray_foreach(astmember, *astnodes) {
        const struct rir_value *argexprval = rir_process_ast_node_getreadval(*astmember, ctx);
        if (!argexprval) {
            RF_ERROR("Could not create rir expression from bracket list member");
            return false;
        }

        if (!(argexprval = rir_maybe_convert(argexprval, member_type, RIRPOS_AST, ctx))) {
            RF_ERROR("Could not create conversion for rir call argument");
            return false;
        }
        darray_append(ret->expr.fixedarr.members, (struct rir_value*)argexprval);
    }

    if (!rir_object_expression_init(ret, RIR_EXPRESSION_FIXEDARR, RIRPOS_AST, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

bool rir_process_bracketlist(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_fixedarr_create_obj_from_ast(n, ctx);
    if (!obj) {
        RF_ERROR("Could not create a rir fixed array instruction");
        return false;
    }
    rir_common_block_add(&ctx->common, &obj->expr);
    RIRCTX_RETURN_EXPR(ctx, true, obj);
}

bool rir_fixedarr_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();

    if (!rf_stringx_append(
            ctx->rir->buff,
            RFS(
                RIRTOSTR_INDENT RFS_PF" = fixedarr(%"PRIu64", "RFS_PF,
                RFS_PA(rir_value_string(&e->val)),
                e->fixedarr.size,
                RFS_PA(rir_type_string(e->fixedarr.member_type))
            )
        )) {
        goto end;
    }


    if (darray_size(e->fixedarr.members) == 0) {
        // TODO
        RF_ASSERT_OR_EXIT(false, "Empty fixedarray members, not implemented yet");
    } else {
        if (!rir_valuearr_tostring_close(ctx, &e->fixedarr.members)) {
            goto end;
        }
        // success
        ret = true;
    }

end:
    RFS_POP();
    return ret;
}

void rir_fixedarr_deinit(struct rir_fixedarr *arr)
{
    darray_free(arr->members);
}



struct rir_object *rir_fixedarrsize_create(struct rir_value *v, struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, rir_ctx_rir(ctx));
    if (!ret) {
        return NULL;
    }
    ret->expr.fixedarrsize.array = v;
    ret->expr.type = RIR_EXPRESSION_FIXEDARRSIZE;
    int64_t size = rir_type_array_size(v->type);
    if (!rir_constantval_init_fromint64(&ret->expr.val, rir_ctx_rir(ctx), size)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

bool rir_fixedarrsize_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();

    if (!rf_stringx_append(
            ctx->rir->buff,
            RFS(
                RIRTOSTR_INDENT RFS_PF" = fixedarrsize("RFS_PF")\n",
                RFS_PA(rir_value_string(&e->val)),
                RFS_PA(rir_value_string(e->fixedarrsize.array))
            )
        )) {
        goto end;
    }

    // success
    ret = true;

end:
    RFS_POP();
    return ret;
}
