#include "llvm_operators.h"

#include <llvm-c/Core.h>
#include <llvm-c/Target.h>

#include <Utils/bits.h>

#include <ast/ast.h>
#include <ast/operators.h>
#include <types/type_elementary.h>
#include <types/type_comparisons.h>
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
            RF_ASSERT(false, "Illegal operand types at equality comparison code generation");
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
            RF_ASSERT(false, "Illegal operand types at unequality comparison code generation");
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
            RF_ASSERT(false, "Illegal operand types at greater than comparison code generation");
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
            RF_ASSERT(false, "Illegal operand types at greater than or equal comparison code generation");
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
            RF_ASSERT(false, "Illegal operand types at less than comparison code generation");
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
            RF_ASSERT(false, "Illegal operand types at less than or equal comparison code generation");
            return NULL;
        }
        break;

    default:
        RF_ASSERT(false, "Illegal binary operation type at comparison code generation");
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
    struct rir_type *defined_type =  rec->rir_data;
    RF_ASSERT(defined_type->category == COMPOSITE_RIR_DEFINED,
              "a member access left hand type can only be a defined type");

    struct rir_type **subtype;
    darray_foreach(subtype, defined_type->subtypes.item[0]->subtypes) {
        if (rf_string_equal(member_s, (*subtype)->name)) {
            LLVMValueRef indices[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), offset, 0) };
            LLVMValueRef gep_to_type = LLVMBuildGEP(ctx->builder, rec->backend_handle, indices, 2, "");
            return LLVMBuildLoad(ctx->builder, gep_to_type, "");
        }
        //else
        offset += 1;
    }

    RF_ASSERT(false, "Typechecking should have made sure no invalid member access exists");
    return NULL;
}

LLVMValueRef bllvm_compile_assign_llvm(LLVMValueRef from,
                                       LLVMValueRef to,
                                       const struct type *type,
                                       enum llvm_assign_options options,
                                       struct llvm_traversal_ctx *ctx)
{
    if (type_is_specific_elementary(type, ELEMENTARY_TYPE_STRING)) {
        bllvm_copy_string(from, to, ctx);
    } else if (type_category_equals(type, TYPE_CATEGORY_DEFINED)) {
        bllvm_memcpy(from, to, ctx);
    } else if (type->category == TYPE_CATEGORY_ELEMENTARY) {
        if (RF_BITFLAG_ON(options, BLLVM_ASSIGN_MATCH_CASE)) {
            // get pointer to the elementary type
            LLVMValueRef from_ptr = LLVMBuildPointerCast(ctx->builder, from, LLVMPointerType(LLVMTypeOf(from), 0), "");
            // memcpy the elementary type to the sum type contents
            bllvm_memcpyn(from_ptr, to, LLVMStoreSizeOfType(ctx->target_data, LLVMTypeOf(from)), ctx);
            return NULL;
        }
        
        bllvm_store(from, to, ctx);
    } else {
        RF_ASSERT(false, "Not yet implemented");
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
    
    return bllvm_compile_assign_llvm(llvm_from, llvm_to, to->expression_type, BLLVM_ASSIGN_SIMPLE, ctx);
}

LLVMValueRef bllvm_compile_bop(struct ast_node *n,
                               struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BINARY_OPERATOR);

    if (ast_binaryop_op(n) == BINARYOP_MEMBER_ACCESS) {
        return bllvm_compile_member_access(n, ctx);
    }

    if (ast_binaryop_op(n) == BINARYOP_ASSIGN) {
        return bllvm_compile_assign(ast_binaryop_right(n),
                                    ast_binaryop_left(n),
                                    n->expression_type,
                                    ctx);
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
        RF_ASSERT(false, "Illegal binary operation type at LLVM code generation");
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
        RF_ASSERT(false, "Illegal unary operation type at LLVM code generation");
        break;
    }
    // error or do nothing
    return operand;
}
