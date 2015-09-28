#include <ir/rir_function.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir_object.h>
#include <ir/rir.h>
#include <ir/rir_strmap.h>
#include <ast/matchexpr.h>
#include <types/type.h>

static bool rir_fndecl_init_args(struct args_arr *argsarr, const struct ast_node *ast_args, bool foreign, struct rir_ctx *ctx)
{
    if (!ast_args) {
        darray_init(*argsarr);
        return true;
    }
    const struct rir_type *arguments = type_get_rir_or_die(ast_node_get_type(ast_args, AST_TYPERETR_AS_LEAF));
    if (rir_type_is_sumtype(arguments)) {
        RFS_PUSH();
        struct rir_typedef *def = rir_typedef_byname(ctx->rir, type_get_unique_type_str(arguments->type, true));
        RFS_POP();
        if (!def) {
            RF_ERROR("Could not find sum type definition in the RIR");
            return false;
        }
        struct rir_object *arg = rir_argument_create_from_typedef(def, ctx);
        darray_init(*argsarr);
        darray_append(*argsarr, arg);

        // also set the rir object in the symbol table if it's not just a foreign function
        if (!foreign) {
            rir_ctx_st_setrecobj(ctx, ast_args, darray_item(*argsarr, 0));
        }
    } else {
        if (!rir_type_to_arg_array(arguments, argsarr, ctx)) {
            RF_ERROR("Could not turn types to function arg array in the RIR");
            return false;
        }
        // no need to create allocas for the arguments, but still need to set them in the rir symbol table if it's not a foreign function
        if (!foreign) {
            struct rir_object **arg;
            darray_foreach(arg, *argsarr) {
                if (!rir_ctx_st_setobj(ctx, (*arg)->arg.name, *arg)) {
                    RF_ERROR("Could not add RIR argument object to function's symbol table");
                    return false;
                }
            }
        }
    }
    return true;
}

static bool rir_fndecl_init(struct rir_fndecl *ret,
                            const struct ast_node *n,
                            struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(ret);
    ret->plain_decl = true;
    ret->name = ast_fndecl_name_str(n);
    struct ast_node *args = ast_fndecl_args_get(n);
    struct ast_node *ast_returns = ast_fndecl_return_get(n);
    if (!rir_fndecl_init_args(&ret->arguments, args, true, ctx)) {
        return false;
    }
    const struct rir_type *return_type = ast_returns
        ? type_get_rir_or_die(ast_node_get_type(ast_returns, AST_TYPERETR_DEFAULT))
        : NULL;

    // if we got a return value allocate space for it. Assume single return values fow now
    // TODO: Take into account multiple return values
    if (return_type) {
        ret->return_type = rir_type_byname(ctx->rir, type_str_or_die(ast_node_get_type(ast_returns, AST_TYPERETR_DEFAULT), TSTR_DEFAULT));
        if (!ret->return_type) {
            RF_ERROR("Could not find function's rir return type");
            return false;
        }
    } else {
        ret->return_type = rir_ltype_elem_create(ELEMENTARY_TYPE_NIL, false);
        if (!ret->return_type) {
            RF_ERROR("Could not find function's rir return type");
            return false;
        }
    }
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

static void rir_fndecl_deinit(struct rir_fndecl *f)
{
    darray_clear(f->arguments);
}

void rir_fndecl_destroy(struct rir_fndecl *f)
{
    rir_fndecl_deinit(f);
    free(f);
}

bool rir_fndecl_nocheck_tostring(struct rirtostr_ctx *ctx, bool is_plain, const struct rir_fndecl *f)
{
    bool ret = false;
    static const struct RFstring close_paren = RF_STRING_STATIC_INIT(")\n");
    static const struct RFstring sep = RF_STRING_STATIC_INIT("; ");
    RFS_PUSH();
    if (!rf_stringx_append(
            ctx->rir->buff,
            RFS("%s("RF_STR_PF_FMT RF_STR_PF_FMT,
                is_plain ? "fndecl" : "fndef",
                RF_STR_PF_ARG(f->name), RF_STR_PF_ARG(&sep)))) {
        goto end;
    }

    if (!rir_argsarr_tostring(ctx, &f->arguments)) {
        return false;
    }

    if (!rf_stringx_append(ctx->rir->buff, &sep)) {
        goto end;
    }

    if (!rf_stringx_append(ctx->rir->buff, rir_ltype_string(f->return_type))){
        goto end;
    }

    if (!rf_stringx_append(ctx->rir->buff, &close_paren)) {
        goto end;
    }

    ret = true;
end:
    RFS_POP();
    return ret;
}
i_INLINE_INS bool rir_fndecl_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *f);








static bool rir_fndef_init(struct rir_fndef *ret,
                              const struct ast_node *n,
                              struct rir_ctx *ctx)
{
    bool success = false;
    RF_STRUCT_ZERO(ret);
    ret->decl.plain_decl = false;
    rir_ctx_reset(ctx);
    ctx->current_fn = ret;
    rir_ctx_push_st(ctx, ast_fnimpl_symbol_table_get(n));
    ret->decl.name = ast_fndecl_name_str(ast_fnimpl_fndecl_get(n));
    darray_init(ret->blocks);
    strmap_init(&ret->map);
    const struct ast_node *decl = ast_fnimpl_fndecl_get(n);
    struct ast_node *args = ast_fndecl_args_get(decl);
    struct ast_node *ast_returns = ast_fndecl_return_get(decl);
    if (!rir_fndecl_init_args(&ret->decl.arguments, args, false, ctx)) {
        goto end;
    }
    const struct rir_type *return_type = ast_returns
        ? type_get_rir_or_die(ast_node_get_type(ast_returns, AST_TYPERETR_DEFAULT))
        : NULL;

    // if we got a return value allocate space for it. Assume single return values fow now
    // TODO: Take into account multiple return values
    if (return_type) {
        const struct RFstring returnval_str = RF_STRING_STATIC_INIT("$returnval");
        ret->decl.return_type = rir_type_byname(ctx->rir, type_str_or_die(ast_node_get_type(ast_returns, AST_TYPERETR_DEFAULT), TSTR_DEFAULT));
        if (!ret->decl.return_type) {
            RF_ERROR("Could not find function's rir return type");
            goto end;
        }
        struct rir_object *alloca = rir_alloca_create_obj(ret->decl.return_type, 1, ctx);
        if (!alloca) {
            goto end;
        }
        if (!rir_map_addobj(ctx, &returnval_str, alloca)) {
            RF_ERROR("Could not add return val to function string map");
            goto end;
        }
    } else {
        ret->decl.return_type = rir_ltype_elem_create(ELEMENTARY_TYPE_NIL, false);
        if (!ret->decl.return_type) {
            RF_ERROR("Could not find function's rir return type");
            goto end;
        }
    }

    // create the end block
    struct rir_block *end_block;
    if (!(end_block = rir_block_functionend_create(ast_returns ? true : false, ctx))) {
        RF_ERROR("Failed to create a RIR function's end block");
        goto end;
    }
    ret->end_label = &end_block->label;

    // finally create the first block of the body
    struct rir_block *first_block  = rir_block_create(ast_fnimpl_body_get(n), true, ctx);
    if (!first_block) {
        RF_ERROR("Failed to turn the body of a function into the RIR format");
        goto end;
    }

    // if during processing a next_block was created but not populated, connect to the end
    if (ctx->next_block && !rir_block_exit_initialized(ctx->next_block)) {
        if (!rir_block_exit_init_branch(&ctx->next_block->exit, ret->end_label)) {
            goto end;
        }
    }

    // if first block of the function does not have an exit, connect it to the end
    if (!rir_block_exit_initialized(first_block)) {
        if (!rir_block_exit_init_branch(&first_block->exit, ret->end_label)) {
            goto end;
        }
    }

    // add the function_end block as last in the block
    rir_fndef_add_block(ret, end_block);

    success = true;
end:
    rir_ctx_pop_st(ctx);
    return success;
}

struct rir_fndef *rir_fndef_create(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_fndef *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndef_init(ret, n, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_fndef_deinit(struct rir_fndef *f)
{
    // not clearing the members of the maps. They should be cleared from the global list
    strmap_clear(&f->map);
    darray_clear(f->blocks);
    rir_fndecl_deinit(&f->decl);
}

void rir_fndef_destroy(struct rir_fndef *f)
{
    rir_fndef_deinit(f);
    free(f);
}

void rir_fndef_add_block(struct rir_fndef *f, struct rir_block *b)
{
    darray_append(f->blocks, b);
}

bool rir_fndef_tostring(struct rirtostr_ctx *ctx, const struct rir_fndef *f)
{
    bool ret = false;
    static const struct RFstring open_curly = RF_STRING_STATIC_INIT("{\n");
    static const struct RFstring close_curly = RF_STRING_STATIC_INIT("}\n");
    // for every function reset the tostring context
    rirtostr_ctx_reset(ctx);

    RFS_PUSH();
    if (!rir_fndecl_tostring(ctx, &f->decl)) {
        goto end;
    }
    if (!rf_stringx_append(ctx->rir->buff, &open_curly)) {
        goto end;
    }

    struct rir_block **b;
    darray_foreach(b, f->blocks) {
        if (!rir_block_tostring(ctx, *b)) {
            goto end;
        }
    }

    if (!rf_stringx_append(ctx->rir->buff, &close_curly)) {
        goto end;
    }
    // success
    ret = true;
end:
    RFS_POP();
    return ret;
}
i_INLINE_INS struct rir_fndef *rir_fndecl_to_fndef(const struct rir_fndecl* d);

bool rir_function_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *d)
{
    if (d->plain_decl) {
        return rir_fndecl_tostring(ctx, d);
    }
    return rir_fndef_tostring(ctx, rir_fndecl_to_fndef(d));
}

void rir_function_destroy(struct rir_fndecl *d)
{
    if (d->plain_decl) {
        rir_fndecl_destroy(d);
    } else {
        rir_fndef_destroy(rir_fndecl_to_fndef(d));
    }
}
