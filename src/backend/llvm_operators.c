#include "llvm_operators.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>

#include <Utils/bits.h>

#include <ast/ast.h>
#include <ast/operators.h>
#include <types/type_elementary.h>
#include <types/type_comparisons.h>
#include <types/type.h>
#include <ir/rir_type.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

static LLVMValueRef bllvm_compile_comparison(struct ast_node *n,
                                             struct llvm_traversal_ctx *ctx)
{
    struct ast_node *left = ast_binaryop_left(n);
    struct ast_node *right = ast_binaryop_right(n);
    LLVMValueRef llvm_left = bllvm_compile_expression(left, ctx,
                                                      RFLLVM_OPTION_IDENTIFIER_VALUE);
    LLVMValueRef llvm_right = bllvm_compile_expression(right, ctx,
                                                       RFLLVM_OPTION_IDENTIFIER_VALUE);
    enum elementary_type_category elementary_type;
    LLVMIntPredicate llvm_int_compare_type;
    LLVMRealPredicate llvm_real_compare_type;
    llvm_left = bllvm_cast_value_to_elementary_maybe(llvm_left,
                                                     ast_binaryop_common_type(n),
                                                     ctx);
    llvm_right = bllvm_cast_value_to_elementary_maybe(llvm_right,
                                                      ast_binaryop_common_type(n),
                                                      ctx);


    elementary_type = type_elementary_get_category(ast_binaryop_common_type(n));
    switch(ast_binaryop_op(n)) {
    case BINARYOP_CMP_EQ:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntEQ;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealOEQ;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at equality comparison code generation");
            return NULL;
        }
        break;

    case BINARYOP_CMP_NEQ:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntNE;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealONE;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at unequality comparison code generation");
            return NULL;
        }
        break;

    case BINARYOP_CMP_GT:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
            llvm_int_compare_type = LLVMIntSGT;
            break;
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntUGT;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealOGT;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at greater than comparison code generation");
            return NULL;
        }
        break;

    case BINARYOP_CMP_GTEQ:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
            llvm_int_compare_type = LLVMIntSGE;
            break;
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntUGE;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealOGE;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at greater than or equal comparison code generation");
            return NULL;
        }
        break;

    case BINARYOP_CMP_LT:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
            llvm_int_compare_type = LLVMIntSLT;
            break;
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntULT;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealOLT;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at less than comparison code generation");
            return NULL;
        }
        break;

    case BINARYOP_CMP_LTEQ:
        switch (elementary_type) {
        case ELEMENTARY_TYPE_CATEGORY_SIGNED:
            llvm_int_compare_type = LLVMIntSLE;
            break;
        case ELEMENTARY_TYPE_CATEGORY_UNSIGNED:
            llvm_int_compare_type = LLVMIntULE;
            break;
        case ELEMENTARY_TYPE_CATEGORY_FLOAT:
            llvm_real_compare_type = LLVMRealOLE;
            break;
        default:
            RF_CRITICAL_FAIL("Illegal operand types at less than or equal comparison code generation");
            return NULL;
        }
        break;

    default:
        RF_CRITICAL_FAIL("Illegal binary operation type at comparison code generation");
        return NULL;
    }

    if (elementary_type == ELEMENTARY_TYPE_CATEGORY_FLOAT) {
        // note: All FCMP operations are ordered for now
        return LLVMBuildFCmp(ctx->builder, llvm_real_compare_type, llvm_left, llvm_right, "");
    }
    // else
    return LLVMBuildICmp(ctx->builder, llvm_int_compare_type, llvm_left, llvm_right, "");
}

static LLVMValueRef bllvm_compile_member_access(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx)
{
    struct symbol_table_record *rec;
    const struct RFstring *owner_type_s = ast_identifier_str(ast_binaryop_left(n));
    const struct RFstring *member_s = ast_identifier_str(ast_binaryop_right(n));
    size_t offset = 0;
    rec = symbol_table_lookup_record(ctx->current_st, owner_type_s, NULL);
    RF_ASSERT(rec->data->category == TYPE_CATEGORY_DEFINED,
              "a member access left hand type can only be a defined type");

    const struct rir_type *rtype = type_get_rir_or_die(rec->data);
    struct rir_type **subtype;
    darray_foreach(subtype, rtype->subtypes.item[0]->subtypes) {
        if (rf_string_equal(member_s, (*subtype)->name)) {
            LLVMValueRef gep_to_member = bllvm_gep_to_struct(rec->backend_handle, offset, ctx);
            return LLVMBuildLoad(ctx->builder, gep_to_member, "");
        }
        //else
        offset += 1;
    }

    // should never get here
    RF_CRITICAL_FAIL("Typechecking should have made sure no invalid member access exists");
    return NULL;
}

LLVMValueRef bllvm_compile_assign_llvm(LLVMValueRef from,
                                       LLVMValueRef to,
                                       const struct type *type,
                                       enum llvm_assign_options options,
                                       struct llvm_traversal_ctx *ctx)
{
    if (type->category == TYPE_CATEGORY_LEAF) {
        bllvm_compile_assign_llvm(from, to, type->leaf.type, options, ctx);
    } else if (type_is_specific_elementary(type, ELEMENTARY_TYPE_STRING)) {
        if (RF_BITFLAG_ON(options, BLLVM_ASSIGN_MATCH_CASE)) {
            from = LLVMBuildBitCast(
                ctx->builder,
                from,
                LLVMPointerType(LLVMGetTypeByName(ctx->llvm_mod, "string"), 0),
                ""
            );
        }
        bllvm_copy_string(from, to, ctx);
    } else if (type_category_equals(type, TYPE_CATEGORY_DEFINED)) {
        bllvm_memcpy(from, to, ctx);
    } else if (type->category == TYPE_CATEGORY_ELEMENTARY) {
        if (RF_BITFLAG_ON(options, BLLVM_ASSIGN_MATCH_CASE)) {
            LLVMTypeRef from_type = LLVMTypeOf(from);
            LLVMValueRef from_ptr = from;
            if (bllvm_type_is_elementary(from_type)) {
                // get pointer to the elementary type
                from_ptr = LLVMBuildPointerCast(ctx->builder, from, LLVMPointerType(from_type, 0), "POINTERTOELEMENTARYCAST");
            }
            // memcpy the elementary type to the sum type contents
            bllvm_memcpyn(from_ptr, to, LLVMStoreSizeOfType(ctx->target_data, LLVMTypeOf(from)), ctx);
        } else {
            bllvm_store(from, to, ctx);
        }
    } else {
        RF_CRITICAL_FAIL("Not yet implemented");
    }

    // hm what should compiling an assignment return?
    return NULL;
}

LLVMValueRef bllvm_compile_assign(struct ast_node *from,
                                  struct ast_node *to,
                                  const struct type *common_type,
                                  struct llvm_traversal_ctx *ctx)
{
    // For left side we want the memory location if it's a simple identifier hence options = 0
    LLVMValueRef llvm_to = bllvm_compile_expression(to, ctx, 0);
    LLVMValueRef llvm_from = bllvm_compile_expression(from,
                                                      ctx,
                                                      RFLLVM_OPTION_IDENTIFIER_VALUE);
    
    return bllvm_compile_assign_llvm(
        llvm_from,
        llvm_to,
        ast_node_get_type(to, AST_TYPERETR_DEFAULT),
        BLLVM_ASSIGN_SIMPLE,
        ctx
    );
}

LLVMValueRef bllvm_compile_bop(struct ast_node *n,
                               struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BINARY_OPERATOR);

    if (ast_binaryop_op(n) == BINARYOP_MEMBER_ACCESS) {
        return bllvm_compile_member_access(n, ctx);
    }

    if (ast_binaryop_op(n) == BINARYOP_ASSIGN) {
        return bllvm_compile_assign(
            ast_binaryop_right(n),
            ast_binaryop_left(n),
            ast_node_get_type(n, AST_TYPERETR_DEFAULT),
            ctx
        );
    }
    LLVMValueRef left = bllvm_compile_expression(ast_binaryop_left(n), ctx, RFLLVM_OPTION_IDENTIFIER_VALUE);
    LLVMValueRef right = bllvm_compile_expression(ast_binaryop_right(n), ctx, RFLLVM_OPTION_IDENTIFIER_VALUE);
    switch(ast_binaryop_op(n)) {
        // arithmetic
        // TODO: This will not be okay for all situations. There are different
        //       functions for different LLVM Types (Floats, Ints, Signed, Unsigned e.t.c.)
        //       Deal with it properly ...
    case BINARYOP_ADD:
        return LLVMBuildAdd(ctx->builder, left, right, "left + right");
    case BINARYOP_SUB:
        return LLVMBuildSub(ctx->builder, left, right, "left - right");
    case BINARYOP_MUL:
        return LLVMBuildMul(ctx->builder, left, right, "left * right");
    case BINARYOP_DIV:
        return LLVMBuildUDiv(ctx->builder, left, right, "left / right");
    case BINARYOP_CMP_EQ:
    case BINARYOP_CMP_NEQ:
    case BINARYOP_CMP_GT:
    case BINARYOP_CMP_GTEQ:
    case BINARYOP_CMP_LT:
    case BINARYOP_CMP_LTEQ:
        return bllvm_compile_comparison(n, ctx);
    default:
        RF_CRITICAL_FAIL("Illegal binary operation type at LLVM code generation");
        break;
    }
    return NULL;
}

LLVMValueRef bllvm_compile_uop(struct ast_node *n,
                               struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_UNARY_OPERATOR);

    LLVMValueRef operand = bllvm_compile_expression(
        ast_unaryop_operand(n),
        ctx,
        RFLLVM_OPTION_IDENTIFIER_VALUE);
    switch(ast_unaryop_op(n)) {
    case UNARYOP_AMPERSAND:
        // TODO: implement
        RF_ASSERT(false, "uop ampersand not yet implemented");
        return operand;
    case UNARYOP_INC:
        // TODO: Distinguish between post/pre
        return LLVMBuildAdd(ctx->builder,
                            operand,
                            LLVMConstInt(LLVMInt32Type(), 1, 0),
                            "uop_inc");
    case UNARYOP_DEC:
        // TODO: Distinguish between post/pre
        return LLVMBuildSub(ctx->builder,
                            operand,
                            LLVMConstInt(LLVMInt32Type(), 1, 0),
                            "uop_inc");
    case UNARYOP_MINUS:
        return LLVMBuildMul(ctx->builder,
                            operand,
                            LLVMConstInt(LLVMInt32Type(), -1, 0),
                            "uop_minus");
    case UNARYOP_PLUS:
        // do nothing? What exactly is a plus unary op doing?
        break;
    default:
        RF_CRITICAL_FAIL("Illegal unary operation type at LLVM code generation");
        break;
    }
    // error or do nothing
    return operand;
}
