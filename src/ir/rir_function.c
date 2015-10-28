#include <ir/rir_function.h>
#include <Utils/memory.h>
#include <ast/ast.h>
#include <ast/function.h>
#include <ast/type.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir_object.h>
#include <ir/rir.h>
#include <ir/rir_strmap.h>
#include <ast/matchexpr.h>
#include <types/type.h>

struct rir_type_fn_ctx {
    struct rir_ctx *rirctx;
};

static inline void rir_type_fn_ctx_init(struct rir_type_fn_ctx *ctx,
                                        struct rir_ctx *rirctx)
{
    ctx->rirctx = rirctx;
}

static bool rir_function_add_variable(struct rir_ltype *ltype, const struct RFstring *name, struct rir_ctx *ctx)
{
    struct rir_object *objvar = rir_variable_create(ltype, ctx);
    if (!objvar) {
        return false;
    }
    darray_append(ctx->current_fn->variables, objvar);
    return rir_ctx_st_setobj(ctx, name, objvar);
}

static bool rir_type_fn_cb(const struct RFstring *name,
                           const struct ast_node *desc,
                           struct type *t,
                           struct rir_type_fn_ctx *ctx)
{
    struct rir_ltype *ltype = rir_ltype_create_from_type(t, ctx->rirctx);
    if (!ltype) {
        return false;
    }
    return rir_function_add_variable(ltype, name, ctx->rirctx);
}

static bool rir_fndecl_init_args(struct rir_type_arr *arr,
                                 struct type *args_type,
                                 const struct ast_node *ast_desc,
                                 bool foreign,
                                 struct rir_ctx *ctx)
{
    if (!args_type) {
        darray_init(*arr);
        return true;
    }

    if (type_is_sumtype(args_type)) {
        RFS_PUSH();
        struct rir_typedef *def = rir_typedef_byname(ctx->rir, type_get_unique_type_str(args_type));
        RFS_POP();
        if (!def) {
            RF_ERROR("Could not find sum type definition in the RIR");
            return false;
        }
        struct rir_ltype *t;
        if (!(t = rir_ltype_comp_create(def, true))) {
            return false;
        }
        darray_init(*arr);
        darray_append(*arr, t);
        if (!rir_function_add_variable(t, type_get_unique_type_str(args_type), ctx)) {
            RF_ERROR("Could not add sum type value to symbol table");
            return false;
        }
    } else {
        if (!rir_typearr_from_type(arr, args_type, ARGARR_AT_FNDECL, ctx)) {
            RF_ERROR("Could not turn types to function arg array in the RIR");
            return false;
        }
        // no need to create allocas for the args_type, but still need to set them in the rir symbol table if it's not a foreign function
        if (!foreign) {
            RF_ASSERT(ast_desc, "An ast type description should have been given at this point");
            struct rir_type_fn_ctx fnctx;
            rir_type_fn_ctx_init(&fnctx, ctx);
            if (!ast_type_foreach_arg(ast_desc, args_type, (ast_type_cb)rir_type_fn_cb, &fnctx)) {
                RF_ERROR("Failed to add rir types to a function's symbol tables");
                return false;
            }
        }
    }
    return true;
}

static bool rir_fndecl_init(struct rir_fndecl *ret,
                            const struct RFstring *name,
                            struct type *arguments,
                            const struct ast_node *ast_args_desc,
                            const struct type *return_type,
                            bool foreign,
                            struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(ret);
    ret->plain_decl = foreign;
    ret->name = name;
    if (!rir_fndecl_init_args(&ret->argument_types, arguments, ast_args_desc, foreign, ctx)) {
        return false;
    }
    if (return_type) {
        ret->return_type = rir_ltype_create_from_type(return_type, ctx);
        if (!ret->return_type) {
            RF_ERROR("Could not find function's rir return type");
            return false;
        }
    } else {
        ret->return_type = rir_ltype_elem_create(ELEMENTARY_TYPE_NIL, false);
        if (!ret->return_type) {
            RF_ERROR("Could not create nil type for a function's return");
            return false;
        }
    }
    return true;
}

struct rir_fndecl *rir_fndecl_create(const struct RFstring *name,
                                     struct type *arguments,
                                     const struct type *return_type,
                                     bool foreign,
                                     struct rir_ctx *ctx)
{
    struct rir_fndecl *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndecl_init(ret, name, arguments, NULL, return_type, foreign, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static bool rir_fndecl_init_from_ast(struct rir_fndecl *ret,
                                     const struct ast_node *n,
                                     struct rir_ctx *ctx)
{
    struct ast_node *ast_returns = ast_fndecl_return_get(n);
    const struct ast_node *ast_args = ast_fndecl_args_get(n);
    bool is_foreign = ast_fndecl_position_get(n) == FNDECL_PARTOF_FOREIGN_IMPORT;
    if (!rir_fndecl_init(
            ret,
            ast_fndecl_name_str(n),
            ast_args ? (struct type*)ast_node_get_type(ast_args) : NULL,
            ast_args,
            ast_returns ? (struct type*)ast_node_get_type(ast_returns) : NULL,
            is_foreign,
            ctx
        )) {
        return false;
    }
    return true;
}

struct rir_fndecl *rir_fndecl_create_from_ast(const struct ast_node *n, struct rir_ctx *ctx)
{
   struct rir_fndecl *ret;
   RF_MALLOC(ret, sizeof(*ret), return NULL);
   if (!rir_fndecl_init_from_ast(ret, n, ctx)) {
       free(ret);
       ret = NULL;
   }
   return ret;
}

static void rir_fndecl_deinit(struct rir_fndecl *f)
{
    darray_free(f->argument_types);
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

    if (!rir_typearr_tostring(ctx, &f->argument_types)) {
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



static inline void rir_fndef_init_common_intro(struct rir_fndef *ret,
                                               struct rir_ctx *ctx)
{
    RF_STRUCT_ZERO(ret);
    rir_ctx_reset(ctx);
    darray_init(ret->variables);
    strmap_init(&ret->map);
    ctx->current_fn = ret;
}

static inline bool rir_fndef_init_common_outro(struct rir_fndef *ret,
                                               const struct type *return_type,
                                               struct rir_ctx *ctx)
{
    darray_init(ret->blocks);
    // if we got a return value allocate space for it. Assume single return values fow now
    // TODO: Take into account multiple return values
    if (return_type) {
        struct rir_expression *alloca = rir_alloca_create(ret->decl.return_type, 1, ctx);
        if (!alloca) {
            return false;
        }
        ret->retslot_expr = alloca;
    }
    return true;
}

static bool rir_fndef_init(struct rir_fndef *ret,
                           const struct RFstring *name,
                           struct type *arguments,
                           const struct type *return_type,
                           struct rir_ctx *ctx)
{
    rir_fndef_init_common_intro(ret, ctx);
    if (!rir_fndecl_init(&ret->decl, name, arguments, NULL, return_type, false, ctx)) {
        return false;
    }
    return rir_fndef_init_common_outro(ret, return_type, ctx);
}

struct rir_fndef *rir_fndef_create(const struct RFstring *name,
                                   struct type *arguments,
                                   const struct type *return_type,
                                   struct rir_ctx *ctx)
{
    struct rir_fndef *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndef_init(ret, name, arguments, return_type, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static bool rir_fndef_init_from_ast(struct rir_fndef *ret,
                                    const struct ast_node *n,
                                    struct rir_ctx *ctx)
{
    rir_fndef_init_common_intro(ret, ctx);
    bool success = false;
    rir_ctx_push_st(ctx, ast_fnimpl_symbol_table_get(n));
    const struct ast_node *decl = ast_fnimpl_fndecl_get(n);
    struct ast_node *ast_returns = ast_fndecl_return_get(decl);
    if (!rir_fndecl_init_from_ast(&ret->decl, decl, ctx)) {
        goto end;
    }
    if (!rir_fndef_init_common_outro(
            ret, ast_returns ? ast_node_get_type(ast_returns) : NULL,
            ctx
        )) {
        goto end;
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

struct rir_fndef *rir_fndef_create_from_ast(const struct ast_node *n, struct rir_ctx *ctx)
{
    struct rir_fndef *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_fndef_init_from_ast(ret, n, ctx)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void rir_fndef_deinit(struct rir_fndef *f)
{
    // not clearing the members of the maps. They should be cleared from the global list
    strmap_clear(&f->map);
    darray_free(f->blocks);
    darray_free(f->variables);
    rir_fndecl_deinit(&f->decl);
}

void rir_fndef_destroy(struct rir_fndef *f)
{
    rir_fndef_deinit(f);
    free(f);
}

int rir_fndef_value_to_argnum(const struct rir_fndef *f, const struct rir_value *v)
{
    struct rir_object **var;
    int idx = 0;
    darray_foreach(var, f->variables) {
        if (v == rir_object_value(*var)) {
            return idx;
        }
        ++idx;
    }
    return -1;
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
