#include "llvm_utils.h"

#include <stdio.h>
#include <llvm-c/Core.h>

#include "llvm_ast.h"

void backend_llvm_val_debug(LLVMValueRef v, const char *val_name)
{
    char *str = LLVMPrintValueToString(v);
    printf("[DEBUG]: Value of \"%s\" is %s\n", val_name, str);
    fflush(stdout);
    LLVMDisposeMessage(str);
}

void backend_llvm_type_debug(LLVMTypeRef t, const char *type_name)
{
    char *str = LLVMPrintTypeToString(t);
    printf("[DEBUG]: Type \"%s\" is %s\n", type_name, str);
    fflush(stdout);
    LLVMDisposeMessage(str);
}

void backend_llvm_mod_debug(LLVMModuleRef m, const char *mod_name)
{
    char *str = LLVMPrintModuleToString(m);
    printf("[DEBUG]: Module \"%s\" is\n %s\n", mod_name, str);
    fflush(stdout);
    LLVMDisposeMessage(str);
}


void backend_llvm_assign_to_string(LLVMValueRef string_alloca,
                                   LLVMValueRef length,
                                   LLVMValueRef string_data,
                                   struct llvm_traversal_ctx *ctx)
{


    // store string length
    LLVMValueRef indices_0[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 0, 0) };
    LLVMValueRef gep_to_strlen = LLVMBuildGEP(ctx->builder, string_alloca, indices_0, 2, "gep_to_str");
    LLVMBuildStore(ctx->builder, length, gep_to_strlen);
    // store string data
    LLVMValueRef indices_1[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 1, 0) };
    LLVMValueRef gep_to_strdata = LLVMBuildGEP(ctx->builder, string_alloca, indices_1, 2, "gep_to_strdata");
    LLVMBuildStore(ctx->builder, string_data, gep_to_strdata);
}

void backend_llvm_load_from_string(LLVMValueRef string_alloca,
                                   LLVMValueRef *length,
                                   LLVMValueRef *string_data,
                                   struct llvm_traversal_ctx *ctx)
{
    // load strlen
    LLVMValueRef indices_0[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 0, 0) };
    LLVMValueRef gep_to_strlen = LLVMBuildGEP(ctx->builder, string_alloca, indices_0, 2, "gep_to_strlen");
    *length = LLVMBuildLoad(ctx->builder, gep_to_strlen, "loaded_str_len");
    // load strdata pointer TODO:load string again?
    LLVMValueRef indices_1[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 1, 0) };
    LLVMValueRef gep_to_strdata = LLVMBuildGEP(ctx->builder, string_alloca, indices_1, 2, "gep_to_strdata");
    *string_data = LLVMBuildLoad(ctx->builder, gep_to_strdata, "loaded_str_data");
}

void backend_llvm_enter_block(struct llvm_traversal_ctx *ctx,
                              struct LLVMOpaqueBasicBlock *block)
{
    LLVMPositionBuilderAtEnd(ctx->builder, block);
    ctx->current_block = block;
}
