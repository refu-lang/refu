#include <ir/rir_function.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir.h>
#include <ir/rir_strmap.h>
#include <types/type.h>

static bool rir_fndecl_init(struct rir_fndecl *ret,
                            const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(ret);
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    ctx->current_fn = ret;
    ret->name = ast_fndecl_name_str(ast_fnimpl_fndecl_get(n));
    strmap_init(&ret->map);
    strmap_init(&ret->id_map);
    struct ast_node *decl = ast_fnimpl_fndecl_get(n);
    struct ast_node *args = ast_fndecl_args_get(decl);
    struct ast_node *returns = ast_fndecl_return_get(decl);
    ret->arguments = args
        ? type_get_rir_or_die(ast_node_get_type(args, AST_TYPERETR_DEFAULT))
        : NULL;
    ret->returns = returns
        ? type_get_rir_or_die(ast_node_get_type(returns, AST_TYPERETR_DEFAULT))
        : NULL;

    if (rir_type_is_sumtype(ret->arguments)) {
        // TODO
    } else {
        if (!rir_type_to_arg_array(ret->arguments, &ret->arguments_list)) {
            return false;
        }
    }


    const struct rir_argument **arg;
    if (rir_type_is_sumtype(ret->arguments)) {
        // TODO
    } else {
        darray_foreach(arg, ret->arguments_list) {
            const struct rir_ltype *arg_type = &(*arg)->type;
            struct rir_expression *e = rir_alloca_create(arg_type, rir_ltype_bytesize(arg_type), ctx);
            strmap_add(&ctx->current_fn->id_map, (*arg)->name, e);
        }
    }

    // finally create the body
    ret->body = rir_block_create(ast_fnimpl_body_get(n), 0, ctx);
    return true;
}

struct rir_fndecl *rir_fndecl_create(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_fndecl *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndecl_init(ret, n, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static bool free_map_member(struct RFstring *id,
                            struct rir_expression *e,
                            void *user_arg)
{
    (void) user_arg;
    rir_expression_destroy(e);
    return true;
}

static void rir_fndecl_deinit(struct rir_fndecl *f)
{
    strmap_iterate(&f->map, free_map_member, NULL);
    strmap_clear(&f->map);
}

void rir_fndecl_destroy(struct rir_fndecl *f)
{
    rir_fndecl_deinit(f);
    free(f);
}

bool rir_fndecl_tostring(struct rir *r, const struct rir_fndecl *f)
{
    bool ret = false;
    static const struct RFstring close_paren = RF_STRING_STATIC_INIT(")");
    static const struct RFstring sep = RF_STRING_STATIC_INIT("; ");

    RFS_PUSH();
    if (!rf_stringx_append(
            r->buff,
            RFS("fndef("RF_STR_PF_FMT RF_STR_PF_FMT,
                RF_STR_PF_ARG(f->name), RF_STR_PF_ARG(&sep)))) {
        goto end;
    }

    if (f->arguments) {
        if (!rf_stringx_append(
            r->buff,
            rir_type_str_or_die(f->arguments)
            )) {
            goto end;
        }
    }

    if (!rf_stringx_append(r->buff, &sep)) {
        goto end;
    }

    if (f->returns) {
        if (!rf_stringx_append(
            r->buff,
            rir_type_str_or_die(f->returns)
            )){
            goto end;
        }
    }

    if (!rf_stringx_append(r->buff, &close_paren)) {
        goto end;
    }

    if (!rir_block_tostring(r, f->body)) {
        goto end;
    }
    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}
