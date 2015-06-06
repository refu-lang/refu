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
#include <analyzer/type_set.h>
#include <analyzer/analyzer.h>
#include <lexer/tokens.h>
#include <types/type.h>
#include <ir/rir_types_list.h>
#include <ir/rir_type.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

#define DEFAULT_PTR_ADDRESS_SPACE 0

static bool bllvm_create_global_types(struct llvm_traversal_ctx *ctx)
{
    struct type *t;
    struct rf_objset_iter it;
    rf_objset_foreach(ctx->a->types_set, &it, t) {
        if (t->category == TYPE_CATEGORY_DEFINED) {
            if (!bllvm_compile_typedecl(type_defined_get_name(t), t, ctx)) {
                return false;
            }
        }
    }
    return true;
}

static LLVMValueRef bllvm_add_global_strbuff(char *str_data, size_t str_len,
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

LLVMValueRef bllvm_create_global_const_string_with_hash(
    const struct RFstring *string,
    uint32_t hash,
    struct llvm_traversal_ctx *ctx)
{
    unsigned int length = rf_string_length_bytes(string);
    struct RFstring *s;

    RFS_PUSH();
    s = RFS_NT_OR_DIE("strbuff_%u", hash);
    LLVMValueRef global_stringbuff = bllvm_add_global_strbuff(
        rf_string_cstr_from_buff_or_die(string),
        length,
        rf_string_data(s),
        ctx
    );

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

    s = RFS_NT_OR_DIE("gstr_%u", hash);
    LLVMValueRef global_val = LLVMAddGlobal(ctx->mod,
                                            LLVMGetTypeByName(ctx->mod, "string"),
                                            rf_string_data(s));
    LLVMSetInitializer(global_val, string_decl);
    RFS_POP();
    return global_val;
}

LLVMValueRef bllvm_create_global_const_string(const struct RFstring *string,
                                              struct llvm_traversal_ctx *ctx)
{
    return bllvm_create_global_const_string_with_hash(
        string,
        rf_hash_str_stable(string, 0),
        ctx
    );
}

static void bllvm_const_string_creation_cb(const struct string_table_record *rec,
                                           struct llvm_traversal_ctx *ctx)
{
    bllvm_create_global_const_string_with_hash(&rec->string, rec->hash, ctx);
}

static void bllvm_create_global_memcpy_decl(struct llvm_traversal_ctx *ctx)
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

static void bllvm_create_global_print_decl(struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef pint_args[] = { LLVMPointerType(LLVMInt64Type(), 0) };
    LLVMAddFunction(ctx->mod, "rf_stdlib_print_int",
                    LLVMFunctionType(LLVMVoidType(),
                                     pint_args,
                                     1,
                                     false));
    LLVMTypeRef pstring_args[] = { LLVMPointerType(LLVMGetTypeByName(ctx->mod, "string"), 0) };
    LLVMAddFunction(ctx->mod, "rf_stdlib_print_string",
                    LLVMFunctionType(LLVMVoidType(),
                                     pstring_args,
                                     1,
                                     false));
}

static void bllcm_create_global_donothing_decl(struct llvm_traversal_ctx *ctx)
{
    // Mainly used for debugging llvm bytecode atm. Maybe remove if not really needed?
    LLVMValueRef fn = LLVMAddFunction(ctx->mod, "llvm.donothing",
                                      LLVMFunctionType(LLVMVoidType(), NULL, 0, false));
    LLVMAddFunctionAttr(fn, LLVMNoUnwindAttribute);
    LLVMAddFunctionAttr(fn, LLVMReadNoneAttribute);
}

static bool bllvm_create_global_functions(struct llvm_traversal_ctx *ctx)
{
    /* -- add printf() declaration -- */
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMAddFunction(ctx->mod, "printf",
                    LLVMFunctionType(LLVMInt32Type(),
                                     printf_args,
                                     1,
                                     true));
    /* -- add exit() -- */
    LLVMTypeRef exit_args[] = { LLVMInt32Type() };
    LLVMAddFunction(ctx->mod, "exit",
                    LLVMFunctionType(LLVMVoidType(),
                                     exit_args,
                                     1,
                                     false));
    /* -- add donothing() -- */
    bllcm_create_global_donothing_decl(ctx);
    /* -- add print() -- */
    bllvm_create_global_print_decl(ctx);
    /* -- add memcpy intrinsic declaration -- */
    bllvm_create_global_memcpy_decl(ctx);
    return true;
}

bool bllvm_create_globals(struct llvm_traversal_ctx *ctx)
{
    // TODO: If possible in llvm these globals creation would be in the beginning
    //       before any module is created or in a global module which would be a
    //       parent of all submodules.
    llvm_traversal_ctx_reset_params(ctx);

    llvm_traversal_ctx_add_param(ctx, LLVMInt32Type());
    llvm_traversal_ctx_add_param(ctx, LLVMPointerType(LLVMInt8Type(), DEFAULT_PTR_ADDRESS_SPACE));
    LLVMTypeRef string_type = LLVMStructCreateNamed(LLVMGetGlobalContext(), "string");
    LLVMStructSetBody(string_type,
                      llvm_traversal_ctx_get_params(ctx),
                      llvm_traversal_ctx_get_param_count(ctx),
                      true);

    llvm_traversal_ctx_reset_params(ctx);
    string_table_iterate(ctx->a->string_literals_table,
                         (string_table_cb)bllvm_const_string_creation_cb, ctx);
    // also add "true" and "false" as global constant string literals
    bllvm_create_global_const_string(tokentype_to_str(TOKEN_KW_TRUE), ctx);
    bllvm_create_global_const_string(tokentype_to_str(TOKEN_KW_FALSE), ctx);


    if (!bllvm_create_global_types(ctx)) {
        RF_ERROR("Could not create global types");
        return false;
    }

    if (!bllvm_create_global_functions(ctx)) {
        RF_ERROR("Could not create global functions");
        return false;
    }
    return true;
}

LLVMValueRef bllvm_get_boolean_str(bool boolean, struct llvm_traversal_ctx *ctx)
{
    // this depends on the string hash being stable and same across all
    // implementations (which it should be)
    return boolean ? LLVMGetNamedGlobal(ctx->mod, "gstr_706834940")
        : LLVMGetNamedGlobal(ctx->mod, "gstr_3855993015");
}
