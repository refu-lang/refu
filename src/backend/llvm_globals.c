#include "llvm_globals.h"

#include <Utils/hash.h>
#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <analyzer/string_table.h>
#include <lexer/tokens.h>
#include <ir/rir_types_list.h>
#include <ir/rir_type.h>
#include <ir/rir.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

#define DEFAULT_PTR_ADDRESS_SPACE 0

static void backend_llvm_create_global_types(struct llvm_traversal_ctx *ctx)
{
    struct rir_type *t;
    rir_types_list_for_each(&ctx->rir->rir_types_list, t) {
        if (t->category == COMPOSITE_RIR_DEFINED) {
            backend_llvm_compile_typedecl(t->name, t, ctx);
        }
    }
}

static LLVMValueRef backend_llvm_add_global_strbuff(char *str_data, size_t str_len,
                                                    char *optional_name,
                                                    struct llvm_traversal_ctx *ctx)
{
    if (!optional_name) {
        optional_name = str_data;
    }
    LLVMValueRef stringbuff = LLVMConstString(str_data, str_len, true);
    LLVMValueRef global_stringbuff = LLVMAddGlobal(ctx->mod, LLVMTypeOf(stringbuff), optional_name);
    LLVMSetInitializer(global_stringbuff, stringbuff);
    LLVMSetUnnamedAddr(global_stringbuff, true);
    LLVMSetLinkage(global_stringbuff, LLVMPrivateLinkage);
    LLVMSetGlobalConstant(global_stringbuff, true);

    return global_stringbuff;
}

LLVMValueRef backend_llvm_create_global_const_string_with_hash(
    const struct RFstring *string,
    uint32_t hash,
    struct llvm_traversal_ctx *ctx)
{
    unsigned int length = rf_string_length_bytes(string);
    char *gstr_name;
    char *strbuff_name;
    struct RFstring *s;

    RFS_PUSH();
    s = RFS("strbuff_%u", hash);
    strbuff_name = rf_string_cstr_from_buff_or_die(s);
    LLVMValueRef global_stringbuff = backend_llvm_add_global_strbuff(
        rf_string_cstr_from_buff_or_die(string),
        length,
        strbuff_name,
        ctx);

    LLVMValueRef indices_0 [] = {
        LLVMConstInt(LLVMInt32Type(), 0, 0),
        LLVMConstInt(LLVMInt32Type(), 0, 0)
    };
    LLVMValueRef gep_to_string_buff = LLVMConstInBoundsGEP(global_stringbuff, indices_0, 2);
    LLVMValueRef string_struct_layout[] = {
        LLVMConstInt(LLVMInt32Type(), length, 0),
        gep_to_string_buff
    };
    LLVMValueRef string_decl = LLVMConstNamedStruct(LLVMGetTypeByName(ctx->mod, "string"),
                                                    string_struct_layout, 2);

    s = RFS("gstr_%u", hash);
    gstr_name = rf_string_cstr_from_buff_or_die(s);
    LLVMValueRef global_val = LLVMAddGlobal(ctx->mod, LLVMGetTypeByName(ctx->mod, "string"), gstr_name);
    LLVMSetInitializer(global_val, string_decl);
    RFS_POP();
    return global_val;
}

LLVMValueRef backend_llvm_create_global_const_string(const struct RFstring *string,
                                                     struct llvm_traversal_ctx *ctx)
{
    return backend_llvm_create_global_const_string_with_hash(
        string,
        rf_hash_str_stable(string, 0),
        ctx);
}

static void backend_llvm_const_string_creation_cb(const struct string_table_record *rec,
                                                  struct llvm_traversal_ctx *ctx)
{
    backend_llvm_create_global_const_string_with_hash(&rec->string, rec->hash, ctx);
}

static void backend_llvm_create_global_memcpy_decl(struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef args[] = { LLVMPointerType(LLVMInt8Type(), 0),
                           LLVMPointerType(LLVMInt8Type(), 0),
                           LLVMInt64Type(),
                           LLVMInt32Type(),
                           LLVMInt1Type() };
    LLVMValueRef fn =  LLVMAddFunction(ctx->mod, "llvm.memcpy.p0i8.p0i8.i64",
                                       LLVMFunctionType(LLVMVoidType(), args, 5, false));

    // adding attributes to the arguments of memcpy as seen when generating llvm code via clang
    //@llvm.memcpy(i8* nocapture, i8* nocapture readonly, i64, i32, i1)
    // TODO: Not sure if these attributes would always work correctly here.
    LLVMAddAttribute(LLVMGetParam(fn, 0), LLVMNoCaptureAttribute);
    LLVMAddAttribute(LLVMGetParam(fn, 1), LLVMNoCaptureAttribute);
    LLVMAddAttribute(LLVMGetParam(fn, 1), LLVMReadOnlyAttribute);
}

static bool backend_llvm_create_global_functions(struct llvm_traversal_ctx *ctx)
{
    /* -- add printf() declaration -- */

    // print() uses clib's printf so we need a declaration for it
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMAddFunction(ctx->mod, "printf",
                              LLVMFunctionType(LLVMInt32Type(),
                                               printf_args,
                                               1,
                                               true));

    /* -- add print() -- */

    LLVMValueRef llvm_fn;
    // evaluating types here since you are not guaranteed order of execution of
    // a function's arguments and this does have sideffects we read from
    LLVMTypeRef  args[] = { LLVMPointerType(LLVMGetTypeByName(ctx->mod, "string"), 0) };
    llvm_fn = LLVMAddFunction(ctx->mod, "print",
                              LLVMFunctionType(LLVMVoidType(),
                                               args,
                                               1,
                                               false));
    // function body
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(llvm_fn, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    // alloca and get variable
    LLVMValueRef alloca = LLVMBuildAlloca(ctx->builder, LLVMTypeOf(LLVMGetParam(llvm_fn, 0)), "string_arg");
    LLVMBuildStore(ctx->builder, LLVMGetParam(llvm_fn, 0), alloca);
    LLVMValueRef loaded_str = LLVMBuildLoad(ctx->builder, alloca, "loaded_str");
    LLVMValueRef length;
    LLVMValueRef string_data;
    backend_llvm_load_from_string(loaded_str, &length, &string_data, ctx);

    // add the "%.*s" global string used by printf to print RFstring
    LLVMValueRef indices_0[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 0, 0) };
    LLVMValueRef printf_str_lit = backend_llvm_add_global_strbuff("%.*s\0", 5, "printf_str_literal", ctx);
    LLVMValueRef gep_to_strlit = LLVMBuildGEP(ctx->builder, printf_str_lit, indices_0, 2, "gep_to_strlit");
    LLVMValueRef printf_call_args[] = { gep_to_strlit, length, string_data };
    LLVMBuildCall(ctx->builder, LLVMGetNamedFunction(ctx->mod, "printf"),
                  printf_call_args,
                  3, "printf_call");

    LLVMBuildRetVoid(ctx->builder);

    /* -- add memcpy intrinsic declaration -- */
    backend_llvm_create_global_memcpy_decl(ctx);
    return true;
}

bool backend_llvm_create_globals(struct llvm_traversal_ctx *ctx)
{
    // TODO: If possible in llvm these globals creation would be in the beginning
    //       before any module is created or in a global module which would be a
    //       parent of all submodules.
    llvm_traversal_ctx_reset_params(ctx);

    llvm_traversal_ctx_add_param(ctx, LLVMInt32Type());
    llvm_traversal_ctx_add_param(ctx, LLVMPointerType(LLVMInt8Type(), DEFAULT_PTR_ADDRESS_SPACE));
    LLVMTypeRef string_type = LLVMStructCreateNamed(LLVMGetGlobalContext(), "string");
    LLVMStructSetBody(string_type, llvm_traversal_ctx_get_params(ctx),
                      llvm_traversal_ctx_get_param_count(ctx), true);

    llvm_traversal_ctx_reset_params(ctx);
    string_table_iterate(ctx->rir->string_literals_table,
                         (string_table_cb)backend_llvm_const_string_creation_cb, ctx);
    // also add "true" and "false" as global constant string literals
    backend_llvm_create_global_const_string(tokentype_to_str(TOKEN_KW_TRUE), ctx);
    backend_llvm_create_global_const_string(tokentype_to_str(TOKEN_KW_FALSE), ctx);


    backend_llvm_create_global_types(ctx);

    if (!backend_llvm_create_global_functions(ctx)) {
        RF_ERROR("Could not create global functions");
        return false;
    }
    return true;
}

LLVMValueRef backend_llvm_get_boolean_str(bool boolean, struct llvm_traversal_ctx *ctx)
{
    // this depends on the string hash being stable and same across all
    // implementations (which it should be)
    return boolean ? LLVMGetNamedGlobal(ctx->mod, "gstr_706834940")
        : LLVMGetNamedGlobal(ctx->mod, "gstr_3855993015");
}
