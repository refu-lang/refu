#include "llvm_globals.h"

#include <rfbase/utils/hash.h>
#include <rfbase/string/common.h>
#include <rfbase/string/conversion.h>
#include <rfbase/string/manipulation.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <module.h>
#include <analyzer/type_set.h>
#include <analyzer/analyzer.h>
#include <lexer/tokens.h>
#include <types/type.h>
#include <ir/rir.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir_typedef.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

#define DEFAULT_PTR_ADDRESS_SPACE 0

bool bllvm_create_module_types(struct rir *r, struct llvm_traversal_ctx *ctx)
{
    struct rir_typedef *def;
    rf_ilist_for_each(&r->typedefs, def, ln) {
        if (!bllvm_compile_typedef(def, ctx)) {
            return false;
        }
    }
    return true;
}

static LLVMValueRef bllvm_add_global_strbuff(char *str_data,
                                             size_t str_len,
                                             char *optional_name,
                                             struct llvm_traversal_ctx *ctx)
{
    if (!optional_name) {
        optional_name = str_data;
    }
    LLVMValueRef stringbuff = LLVMConstStringInContext(
        ctx->llvm_context,
        str_data,
        str_len,
        true
    );
    LLVMValueRef global_stringbuff = LLVMAddGlobal(
        ctx->llvm_mod,
        LLVMTypeOf(stringbuff),
        optional_name
    );
    LLVMSetInitializer(global_stringbuff, stringbuff);
    LLVMSetUnnamedAddr(global_stringbuff, true);
    LLVMSetLinkage(global_stringbuff, LLVMPrivateLinkage);
    LLVMSetGlobalConstant(global_stringbuff, true);

    return global_stringbuff;
}

LLVMValueRef bllvm_create_global_const_string_with_hash(
    const struct RFstring *string_name,
    const struct RFstring *string_val,
    uint32_t hash,
    struct llvm_traversal_ctx *ctx)
{
    unsigned int length = rf_string_length_bytes(string_val);
    struct RFstring *s;
    RFS_PUSH();
    s = RFS_NT_OR_DIE("strbuff_%u", hash);
    LLVMValueRef global_stringbuff = bllvm_add_global_strbuff(
        rf_string_cstr_from_buff_or_die(string_val),
        length,
        rf_string_data(s),
        ctx
    );

    LLVMValueRef indices_0 [] = {
        LLVMConstInt(LLVMInt32TypeInContext(ctx->llvm_context), 0, 0),
        LLVMConstInt(LLVMInt32TypeInContext(ctx->llvm_context), 0, 0)
    };
    LLVMValueRef gep_to_string_buff = LLVMConstInBoundsGEP(global_stringbuff, indices_0, 2);
    LLVMValueRef string_struct_layout[] = {
        LLVMConstInt(LLVMInt32TypeInContext(ctx->llvm_context), length, 0),
        gep_to_string_buff
    };
    LLVMValueRef string_decl = LLVMConstNamedStruct(
        LLVMGetTypeByName(ctx->llvm_mod, "string"),
        string_struct_layout,
        2
    );

    LLVMValueRef global_val = LLVMAddGlobal(
        ctx->llvm_mod,
        LLVMGetTypeByName(ctx->llvm_mod, "string"),
        rf_string_cstr_from_buff_or_die(string_name)
    );
    LLVMSetInitializer(global_val, string_decl);
    RFS_POP();
    return global_val;
}

LLVMValueRef bllvm_create_global_const_string(const struct RFstring *string_name,
                                              const struct RFstring *string_val,
                                              struct llvm_traversal_ctx *ctx)
{
    return bllvm_create_global_const_string_with_hash(
        string_name,
        string_val,
        rf_hash_str_stable(string_val, 0),
        ctx
    );
}

struct LLVMOpaqueValue *bllvm_literal_to_global_string(const struct RFstring *lit,
                                                       struct llvm_traversal_ctx *ctx)
{
    struct RFstring *s;
    RFS_PUSH();
    s = RFS_NT_OR_DIE("gstr_%u", rf_hash_str_stable(lit, 0));
    LLVMValueRef ret = LLVMGetNamedGlobal(ctx->llvm_mod, rf_string_data(s));
    RFS_POP();
    if (!ret) {
        RF_ERROR("Failed to retrieve a global string from a literal in LLVM");
    }
    return ret;
}

static void bllvm_create_global_memcpy_decl(struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef args[] = {
        LLVMPointerType(LLVMInt8TypeInContext(ctx->llvm_context), 0),
        LLVMPointerType(LLVMInt8TypeInContext(ctx->llvm_context), 0),
        LLVMInt64TypeInContext(ctx->llvm_context),
        LLVMInt32TypeInContext(ctx->llvm_context),
        LLVMInt1TypeInContext(ctx->llvm_context)
    };
    LLVMValueRef fn =  LLVMAddFunction(
        ctx->llvm_mod,
        "llvm.memcpy.p0i8.p0i8.i64",
        LLVMFunctionType(LLVMVoidTypeInContext(ctx->llvm_context), args, 5, false)
    );

    // adding attributes to the arguments of memcpy as seen when generating llvm code via clang
    //@llvm.memcpy(i8* nocapture, i8* nocapture readonly, i64, i32, i1)
    // TODO: Not sure if these attributes would always work correctly here.
    LLVMAddAttribute(LLVMGetParam(fn, 0), LLVMNoCaptureAttribute);
    LLVMAddAttribute(LLVMGetParam(fn, 1), LLVMNoCaptureAttribute);
    LLVMAddAttribute(LLVMGetParam(fn, 1), LLVMReadOnlyAttribute);
}

static void bllvm_create_global_donothing_decl(struct llvm_traversal_ctx *ctx)
{
    // Mainly used for debugging llvm bytecode atm. Maybe remove if not really needed?
    LLVMValueRef fn = LLVMAddFunction(
        ctx->llvm_mod,
        "llvm.donothing",
        LLVMFunctionType(LLVMVoidTypeInContext(ctx->llvm_context), NULL, 0, false));
    LLVMAddFunctionAttr(fn, LLVMNoUnwindAttribute);
    LLVMAddFunctionAttr(fn, LLVMReadNoneAttribute);
}

bool bllvm_create_global_functions(struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef rc;
    /* -- add printf() declaration -- */
    LLVMTypeRef printf_args[] = {
        LLVMPointerType(LLVMInt8TypeInContext(ctx->llvm_context), 0)
    };
    rc = LLVMAddFunction(
        ctx->llvm_mod,
        "printf",
        LLVMFunctionType(LLVMInt32TypeInContext(ctx->llvm_context), printf_args, 1, true)
    );
    RF_ASSERT(rc, "Failed to add printf() as a global LLVM function");
    /* -- add exit() -- */
    LLVMTypeRef exit_args[] = { LLVMInt32TypeInContext(ctx->llvm_context) };
    rc = LLVMAddFunction(
        ctx->llvm_mod,
        "exit",
        LLVMFunctionType(LLVMVoidTypeInContext(ctx->llvm_context), exit_args, 1, false)
    );
    RF_ASSERT(rc, "Failed to add exit() as a global LLVM function");
    /* -- add malloc -- */
    LLVMTypeRef malloc_args[] = { LLVMInt64TypeInContext(ctx->llvm_context) };
    LLVMTypeRef malloc_ret = LLVMPointerType(LLVMInt8TypeInContext(ctx->llvm_context), 0);
    rc = LLVMAddFunction(
        ctx->llvm_mod,
        "malloc",
        LLVMFunctionType(malloc_ret, malloc_args, 1, false)
    );
    RF_ASSERT(rc, "Failed to add malloc() as a global LLVM function");
    /* -- add donothing() -- */
    bllvm_create_global_donothing_decl(ctx);
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

    llvm_traversal_ctx_add_param(ctx, LLVMInt32TypeInContext(ctx->llvm_context));
    llvm_traversal_ctx_add_param(
        ctx,
        LLVMPointerType(LLVMInt8TypeInContext(ctx->llvm_context), DEFAULT_PTR_ADDRESS_SPACE)
    );
    LLVMTypeRef string_type = LLVMStructCreateNamed(ctx->llvm_context, "string");
    /* LLVMTypeRef string_type = LLVMStructCreateNamed(LLVMGetGlobalContext(), "string"); */
    LLVMStructSetBody(string_type,
                      llvm_traversal_ctx_get_params(ctx),
                      llvm_traversal_ctx_get_param_count(ctx),
                      true);

    // In the past we used to have the global function creations (like malloc,
    // memcpy, exit) here. Unfortunately after llvm 3.8 LLVMLinkModules() no
    // longer passes them to the linked module so we are calling it for every
    // module now.

    return true;
}

static bool iterate_literals_cb(const struct RFstring *member, struct rir_object *obj, struct llvm_traversal_ctx *ctx)
{
    struct rir_global *g = &obj->global;
    RF_ASSERT(
        rir_type_is_specific_elementary(rir_global_type(g), ELEMENTARY_TYPE_STRING),
        "Found non string global in string literals map"
    );
    RFS_PUSH();
    bllvm_create_global_const_string(
        // skip the initial '$'
        rf_string_prune_start(&g->val.id, 1, RF_SOPT_ASCII | RF_SOPT_TMP, NULL),
        &g->val.literal,
        ctx
    );
    RFS_POP();
    return true;
}

bool bllvm_create_module_globals(struct rir *r, struct llvm_traversal_ctx *ctx)
{
    // iterate string literals maps to create all constant strings
    strmap_iterate(&r->global_literals, (strmap_it_cb)iterate_literals_cb, ctx);
    return true;
}

LLVMValueRef bllvm_get_boolean_str(bool boolean, struct llvm_traversal_ctx *ctx)
{
    // this depends on the string hash being stable and same across all
    // implementations (which it should be)
    return boolean ? LLVMGetNamedGlobal(ctx->llvm_mod, "gstr_706834940")
        : LLVMGetNamedGlobal(ctx->llvm_mod, "gstr_3855993015");
}
