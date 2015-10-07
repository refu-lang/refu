#include <ir/rir_binaryop.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_expression.h>
#include <ir/rir_block.h>
#include <ir/rir_typedef.h>
#include <ir/rir_constant.h>
#include <ir/rir_process.h>
#include <ir/rir_utils.h>
#include <ast/operators.h>
#include <types/type.h>


static const enum rir_expression_type binaryop_operation_to_rir[] = {
    [BINARYOP_ADD]               =   RIR_EXPRESSION_ADD,
    [BINARYOP_SUB]               =   RIR_EXPRESSION_SUB,
    [BINARYOP_MUL]               =   RIR_EXPRESSION_MUL,
    [BINARYOP_DIV]               =   RIR_EXPRESSION_DIV,

    [BINARYOP_CMP_EQ]            =   RIR_EXPRESSION_CMP_EQ,
    [BINARYOP_CMP_NEQ]           =   RIR_EXPRESSION_CMP_NE,
    [BINARYOP_CMP_GT]            =   RIR_EXPRESSION_CMP_GT,
    [BINARYOP_CMP_GTEQ]          =   RIR_EXPRESSION_CMP_GE,
    [BINARYOP_CMP_LT]            =   RIR_EXPRESSION_CMP_LT,
    [BINARYOP_CMP_LTEQ]          =   RIR_EXPRESSION_CMP_LE,

    [BINARYOP_LOGIC_AND]         =   RIR_EXPRESSION_LOGIC_AND,
    [BINARYOP_LOGIC_OR]          =   RIR_EXPRESSION_LOGIC_OR,
    [BINARYOP_ASSIGN]            =   RIR_EXPRESSION_WRITE,
};

enum binaryop_type rir_binaryop_type_from_ast(const struct ast_binaryop *op)
{
    return binaryop_operation_to_rir[op->type];
}

static inline bool rir_binaryop_init(struct rir_binaryop *op,
                                     const struct rir_value *a,
                                     const struct rir_value *b,
                                     struct rir_ctx *ctx)
{
    struct rir_expression *e;
    // for operations on pointers, first create a read from memory
    if (a->type->is_pointer) {
        if (!(e = rir_read_create(a, ctx))) {
            return false;
        }
        rirctx_block_add(ctx, e);
        a = &e->val;
    }
    if (b->type->is_pointer) {
        if (!(e = rir_read_create(b, ctx))) {
            return false;
        }
        rirctx_block_add(ctx, e);
        b = &e->val;
    }
    op->a = a;
    op->b = b;
    return true;
}

struct rir_object *rir_binaryop_create_nonast_obj(enum rir_expression_type type,
                                                  const struct rir_value *a,
                                                  const struct rir_value *b,
                                                  struct rir_ctx *ctx)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_EXPRESSION, ctx->rir);
    if (!ret) {
        goto fail;
    }
    if (!rir_binaryop_init(&ret->expr.binaryop, a, b, ctx)) {
        goto fail;
    }
    if (!rir_expression_init(ret, type, ctx)) {
        goto fail;
    }
    return ret;

fail:
    free(ret);
    return NULL;
}

struct rir_expression *rir_binaryop_create_nonast(enum rir_expression_type type,
                                                  const struct rir_value *a,
                                                  const struct rir_value *b,
                                                  struct rir_ctx *ctx)
{
    struct rir_object *obj = rir_binaryop_create_nonast_obj(type, a, b, ctx);
    return obj ? &obj->expr : NULL;
}

struct rir_expression *rir_binaryop_create(const struct ast_binaryop *op,
                                           const struct rir_value *a,
                                           const struct rir_value *b,
                                           struct rir_ctx *ctx)
{
    return rir_binaryop_create_nonast(rir_binaryop_type_from_ast(op), a, b, ctx);
}

struct rir_object *rir_binaryop_create_obj(const struct ast_binaryop *op,
                                           const struct rir_value *a,
                                           const struct rir_value *b,
                                           struct rir_ctx *ctx)
{
    return rir_binaryop_create_nonast_obj(rir_binaryop_type_from_ast(op), a, b, ctx);
}

static bool rir_process_memberaccess(const struct ast_binaryop *op,
                                     struct rir_ctx *ctx)
{
    if (op->left->type != AST_IDENTIFIER) {
        RF_ERROR("Left part of a member access was not an identifier");
        goto fail;
    }
    if (!rir_process_identifier(op->left, ctx)) {
        goto fail;
    }
    const struct rir_value *lhs_val = rir_ctx_lastval_get(ctx);
    const struct RFstring *rightstr = ast_identifier_str(op->right);
    const struct type *owner_type = ast_node_get_type_or_die(op->left, AST_TYPERETR_DEFAULT);
    struct rir_typedef *def = rir_typedef_frommap(ctx->rir, type_defined_get_name(owner_type));
    if (!def) {
        RF_ERROR("Could not find rir typedef for a member access type");
        goto fail;
    }

    // find the index of the right part of member access
    struct rir_object **arg;
    unsigned int index = 0;
    darray_foreach(arg, def->arguments_list) {
        if (rf_string_equal(rightstr, (*arg)->arg.name)) {
            break;
        }
        ++index;
    }
    if (index == darray_size(def->arguments_list)) {
        RF_ERROR("Could not find argument in typedef");
        goto fail;
    }

    // create a rir expression to read the object value at the assignee's index position
    struct rir_object *member_ptr_obj = rir_objmemberat_create_obj(lhs_val, index, ctx);
    if (!member_ptr_obj) {
        goto fail;
    }
    rirctx_block_add(ctx, &member_ptr_obj->expr);
    member_ptr_obj = rir_getread_obj(member_ptr_obj, ctx);
    if (!member_ptr_obj) {
        goto fail;
    }

    // return the memberobjat to be used by other rir expressions
    RIRCTX_RETURN_EXPR(ctx, true, member_ptr_obj);

fail:
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

bool rir_process_binaryop(const struct ast_binaryop *op,
                          struct rir_ctx *ctx)
{
    struct rir_object *obj = NULL;
    // special treatment for member access
    if (op->type == BINARYOP_MEMBER_ACCESS) {
        return rir_process_memberaccess(op, ctx);
    }

    const struct rir_value *lval = rir_process_ast_node_getval(op->left, ctx);
    if (!lval) {
        RF_ERROR("A left value should have been created for a binary operation");
        goto fail;
    }
    if (op->type == BINARYOP_ASSIGN) { // set the last assigned to object
        ctx->last_assign_obj = ctx->returned_obj;
    }
    struct rir_value *rval = rir_process_ast_node_getval(op->right, ctx);
    if (!rval) {
        goto fail;
    }
    if (op->right->type == AST_FUNCTION_CALL) {
        // for function call rhs all of the writting should have already been done
        RIRCTX_RETURN_EXPR(ctx, true, NULL);
    }

    if (op->type == BINARYOP_ASSIGN) { // assignment is a special case
        if (op->right->type == AST_MATCH_EXPRESSION) {
            // for assignments from a match expression we should be done
            RIRCTX_RETURN_EXPR(ctx, true, NULL);
        }
        // else, create a rir_write
        if (!(obj = rir_write_create_obj(lval, rval, ctx))) {
            goto fail;
        }
    } else { // normal binary operator processing
        if (!(obj = rir_binaryop_create_obj(op, lval, rval, ctx))) {
            RF_ERROR("Failed to create a rir binary operation");
            goto fail;
        }
    }

    rirctx_block_add(ctx, &obj->expr);
    ctx->last_assign_obj = NULL;
    RIRCTX_RETURN_EXPR(ctx, true, obj);

fail:
    ctx->last_assign_obj = NULL;
    RIRCTX_RETURN_EXPR(ctx, false, NULL);
}

static const struct RFstring rir_bop_type_strings[] = {
    [RIR_EXPRESSION_ADD] = RF_STRING_STATIC_INIT("add"),
    [RIR_EXPRESSION_SUB] = RF_STRING_STATIC_INIT("sub"),
    [RIR_EXPRESSION_MUL] = RF_STRING_STATIC_INIT("mul"),
    [RIR_EXPRESSION_DIV] = RF_STRING_STATIC_INIT("div"),
    [RIR_EXPRESSION_CMP_EQ] = RF_STRING_STATIC_INIT("cmpeq"),
    [RIR_EXPRESSION_CMP_NE] = RF_STRING_STATIC_INIT("cmpne"),
    [RIR_EXPRESSION_CMP_GT] = RF_STRING_STATIC_INIT("cmpgt"),
    [RIR_EXPRESSION_CMP_GE] = RF_STRING_STATIC_INIT("cmpge"),
    [RIR_EXPRESSION_CMP_LT] = RF_STRING_STATIC_INIT("cmplt"),
    [RIR_EXPRESSION_CMP_LE] = RF_STRING_STATIC_INIT("cmple"),
};

bool rir_binaryop_tostring(struct rirtostr_ctx *ctx, const struct rir_expression *e)
{
    bool ret = false;
    RFS_PUSH();

    const struct RFstring *memtype_s = rir_ltype_string(e->binaryop.a->type);
    if (e->val.category == RIR_VALUE_NIL) {
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT"(" RF_STR_PF_FMT ", "RF_STR_PF_FMT ", " RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(&rir_bop_type_strings[e->type]),
                    RF_STR_PF_ARG(memtype_s),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.a)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.b)))
            )) {
            goto end;
        }
    } else {
        if (!rf_stringx_append(
                ctx->rir->buff,
                RFS(RIRTOSTR_INDENT RF_STR_PF_FMT" = "RF_STR_PF_FMT"(" RF_STR_PF_FMT ", "RF_STR_PF_FMT ", " RF_STR_PF_FMT ")\n",
                    RF_STR_PF_ARG(rir_value_string(&e->val)),
                    RF_STR_PF_ARG(&rir_bop_type_strings[e->type]),
                    RF_STR_PF_ARG(memtype_s),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.a)),
                    RF_STR_PF_ARG(rir_value_string(e->binaryop.b)))
            )) {
            goto end;
        }
    }

    ret = true;
end:
    RFS_POP();
    return ret;
}
