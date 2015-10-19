#include <ast/type.h>
#include <ast/ast.h>
#include <types/type.h>

struct ast_type_traversal_ctx {
    //! Current type operation while iterating the rir type to type comparison
    enum typeop_type current_op;
    //! A stack of the currently visited rir types
    struct {darray(struct type*);} visited_types;
    //! A stack of the indices of the currently visited rir types. -1 for uninitialized
    darray(int) indices;
    //! Pointer to the identifier's string (left part) of the last ast typeleaf
    const struct RFstring *last_name;
    //! The callback to call during the traversal
    ast_type_cb callback;
    //! The optional user argument to the callback
    void *user_arg;
};

static inline void ast_type_traversal_ctx_push_type(struct ast_type_traversal_ctx *ctx,
                                                    struct type *type)
{
    darray_push(ctx->visited_types, type);
    darray_push(ctx->indices, -1);
}

static inline struct type *ast_type_traversal_ctx_current_type(struct ast_type_traversal_ctx *ctx)
{
    struct type *top_type = darray_top(ctx->visited_types);
    if (darray_top(ctx->indices) == -1) {
        return top_type;
    }
    // at this point since we have an index the top type should be an operator
    if (top_type->category != TYPE_CATEGORY_OPERATOR) {
        return NULL;
    }
    if (darray_top(ctx->indices) >= (int)darray_size(top_type->operator.operands)) {
        return NULL;
    }
    return darray_item(top_type->operator.operands, darray_top(ctx->indices));
}

static inline void ast_type_traversal_ctx_idx_plus1(struct ast_type_traversal_ctx *ctx)
{
    darray_top(ctx->indices) = darray_top(ctx->indices) + 1;
}

static inline void ast_type_traversal_ctx_go_up(struct ast_type_traversal_ctx *ctx, const struct ast_node *n)
{
    // this should only be called at the end of a typeop traversal
    RF_ASSERT(n->type == AST_TYPE_OPERATOR, "Expected ast type operator");
    enum typeop_type op = ast_typeop_op(n);
    if (op != ctx->current_op) {
        (void)darray_pop(ctx->indices);
        (void)darray_pop(ctx->visited_types);

        // also go to next index of the previous type
        ctx->current_op = op;
        ast_type_traversal_ctx_idx_plus1(ctx);
    }
}

static void ast_type_traversal_ctx_init(struct ast_type_traversal_ctx *ctx, struct type *start_t, ast_type_cb cb, void *user)
{
    darray_init(ctx->visited_types);
    darray_init(ctx->indices);
    ast_type_traversal_ctx_push_type(ctx, start_t);
    ctx->current_op = TYPEOP_INVALID;
    ctx->last_name = NULL;
    ctx->callback = cb;
    ctx->user_arg = user;
}

static void ast_type_traversal_ctx_deinit(struct ast_type_traversal_ctx *ctx)
{
    darray_free(ctx->visited_types);
    darray_free(ctx->indices);
}

static bool ast_type_operator_traversal_pre(const struct ast_node *n, struct ast_type_traversal_ctx *ctx)
{
    struct type *current_t = ast_type_traversal_ctx_current_type(ctx);
    if (!current_t) {
        return false;
    }
    enum typeop_type n_type_op = ast_typeop_op(n);
    if (ctx->current_op == TYPEOP_INVALID) {
        // if it's the first typeop encountered in this traversal
        ctx->current_op = n_type_op;
        if (type_typeop_get(current_t) == TYPEOP_INVALID) {
            // current type is not a type operator
            return false;
        }
        // also go to the first operand of the type
        ast_type_traversal_ctx_idx_plus1(ctx);
    } else {
        // check if we need to go deeper into the type
        if (n_type_op != ctx->current_op) {
            ast_type_traversal_ctx_push_type(ctx, current_t);
            // also go to the first index of the new type
            ctx->current_op = n_type_op;
            ast_type_traversal_ctx_idx_plus1(ctx);
        }
    }
    return true;
}

static bool ast_type_foreach_arg_do(const struct ast_node *n, struct ast_type_traversal_ctx *ctx)
{
    switch(n->type) {
    case AST_TYPE_LEAF:
        AST_NODE_ASSERT_TYPE(ast_typeleaf_left(n), AST_IDENTIFIER);
        ctx->last_name = ast_identifier_str(ast_typeleaf_left(n));
        if (!ast_type_foreach_arg_do(ast_typeleaf_right(n), ctx)) {
            return false;
        }
        break;
    case AST_TYPE_OPERATOR:

        if (!ast_type_foreach_arg_do(ast_typeop_left(n), ctx)) {
            return false;
        }
        if (!ast_type_operator_traversal_pre(n, ctx)) {
            return false;
        }
        if (!ast_type_foreach_arg_do(ast_typeop_right(n), ctx)) {
            return false;
        }
        ast_type_traversal_ctx_go_up(ctx, n);
        break;
    case AST_TYPE_DESCRIPTION:
        return ast_type_foreach_arg_do(ast_typedesc_desc_get(n) , ctx);
    case AST_TYPE_DECLARATION:
        return ast_type_foreach_arg_do(ast_typedecl_typedesc_get(n) , ctx);
    case AST_XIDENTIFIER:
        if (ctx->callback) {
            if (!ctx->callback(ctx->last_name, n, ast_type_traversal_ctx_current_type(ctx), ctx->user_arg)) {
                return false;
            }
        }
        ast_type_traversal_ctx_idx_plus1(ctx);
        break;
    default:
        RF_CRITICAL_FAIL("Should never occur");
        return false;
    }
    return true;
}

bool ast_type_foreach_arg(const struct ast_node *n, struct type *t, ast_type_cb cb, void *user)
{
    struct ast_type_traversal_ctx ctx;
    ast_type_traversal_ctx_init(&ctx, t, cb, user);
    bool ret = ast_type_foreach_arg_do(n, &ctx);
    ast_type_traversal_ctx_deinit(&ctx);
    return ret;
}
