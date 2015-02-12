#include "llvm_ast.h"

#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>
#include <Utils/sanity.h>

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <compiler_args.h>

#include <ast/ast_utils.h>
#include <ast/function.h>
#include <ast/type.h>
#include <ast/block.h>
#include <ast/returnstmt.h>
#include <ast/constant_num.h>
#include <ast/operators.h>
#include <ast/vardecl.h>
#include <ast/string_literal.h>

#include <types/type_function.h>
#include <types/type_elementary.h>

#include <analyzer/string_table.h>

#include <ir/elements.h>
#include <ir/rir_type.h>
#include <ir/rir.h>

#include <backend/llvm.h>
#include "llvm_utils.h"

#define DEFAULT_PTR_ADDRESS_SPACE 0

static LLVMTypeRef backend_llvm_elementary_to_type(enum rir_type_category type, struct llvm_traversal_ctx *ctx)
{
    switch(type) {
        // LLVM does not differentiate between signed and unsigned
    case ELEMENTARY_RIR_TYPE_INT:
    case ELEMENTARY_RIR_TYPE_UINT:
        return LLVMIntType(32);// TODO: Think of how to represent size agnostic
    case ELEMENTARY_RIR_TYPE_INT_8:
    case ELEMENTARY_RIR_TYPE_UINT_8:
        return LLVMInt8Type();
    case ELEMENTARY_RIR_TYPE_INT_16:
    case ELEMENTARY_RIR_TYPE_UINT_16:
        return LLVMInt16Type();
    case ELEMENTARY_RIR_TYPE_INT_32:
    case ELEMENTARY_RIR_TYPE_UINT_32:
        return LLVMInt32Type();
    case ELEMENTARY_RIR_TYPE_INT_64:
    case ELEMENTARY_RIR_TYPE_UINT_64:
        return LLVMInt64Type();

    case ELEMENTARY_RIR_TYPE_STRING:
        return LLVMGetTypeByName(ctx->mod, "string");

    case ELEMENTARY_RIR_TYPE_NIL:
        return LLVMVoidType();

    default:
        RF_ASSERT_OR_CRITICAL(false,
                              "Unsupported elementary type \""RF_STR_PF_FMT"\" "
                              "during LLVM conversion",
                              RF_STR_PF_ARG(type_elementary_get_str((enum elementary_type)type)));
        break;
    }
    return NULL;
}

i_INLINE_INS struct LLVMOpaqueType **llvm_traversal_ctx_get_params(struct llvm_traversal_ctx *ctx);
i_INLINE_INS unsigned llvm_traversal_ctx_get_param_count(struct llvm_traversal_ctx *ctx);
i_INLINE_INS void llvm_traversal_ctx_reset_params(struct llvm_traversal_ctx *ctx);
void llvm_traversal_ctx_add_param(struct llvm_traversal_ctx *ctx,
                                  struct LLVMOpaqueType *type)
{
    darray_append(ctx->params, type);
}

static LLVMTypeRef backend_llvm_type(const struct rir_type *type,
                                     struct llvm_traversal_ctx *ctx)
{
    return backend_llvm_elementary_to_type(type->category, ctx);
}

// return an array of arg types or NULL if our param type is nil
static LLVMTypeRef *backend_llvm_fn_arg_types(struct rir_type *type,
                                              struct llvm_traversal_ctx *ctx)
{
    struct rir_type **subtype;
    LLVMTypeRef llvm_type;
    llvm_traversal_ctx_reset_params(ctx);
    if (darray_size(type->subtypes) == 0) {
        llvm_type = backend_llvm_type(type, ctx);
        if (llvm_type != LLVMVoidType()) {
            llvm_traversal_ctx_add_param(ctx, llvm_type);
        }
    } else {
        darray_foreach(subtype, type->subtypes) {
            llvm_type = backend_llvm_type(*subtype, ctx);
            if (llvm_type != LLVMVoidType()) {
                llvm_traversal_ctx_add_param(ctx, llvm_type);
            }
        }
    }
    return darray_size(ctx->params) == 0 ? NULL : llvm_traversal_ctx_get_params(ctx);
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

static LLVMValueRef backend_llvm_expresion_compile_assign(struct ast_node *n,
                                                          LLVMValueRef left,
                                                          LLVMValueRef right,
                                                          struct llvm_traversal_ctx *ctx)
{
    // for now only assignments to string literals
    AST_NODE_ASSERT_TYPE(ast_binaryop_right(n), AST_STRING_LITERAL);

    LLVMValueRef length;
    LLVMValueRef string_data;
    backend_llvm_load_from_string(right, &length, &string_data, ctx);
    backend_llvm_assign_to_string(left, length, string_data, ctx);

    // hm what should compiling an assignment return?
    return NULL;
}

static LLVMValueRef backend_llvm_expression_compile_bop(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BINARY_OPERATOR);
    LLVMValueRef left = backend_llvm_expression_compile(ast_binaryop_left(n), ctx);
    LLVMValueRef right = backend_llvm_expression_compile(ast_binaryop_right(n), ctx);
    switch(ast_binaryop_op(n)) {
        // arithmetic
        // note: There are different LLVMBuild Arithmetic functions. See if some should
        //       be used in special situations
    case BINARYOP_ADD:
        return LLVMBuildAdd(ctx->builder, left, right, "left + right");
    case BINARYOP_SUB:
        return LLVMBuildSub(ctx->builder, left, right, "left - right");
    case BINARYOP_MUL:
        return LLVMBuildMul(ctx->builder, left, right, "left * right");
    case BINARYOP_DIV:
        return LLVMBuildUDiv(ctx->builder, left, right, "left / right");
    case BINARYOP_ASSIGN:
        return backend_llvm_expresion_compile_assign(n, left, right, ctx);
    default:
        RF_ASSERT(false, "Illegal binary operation type at LLVM code generation");
        break;
    }
    return NULL;
}

LLVMValueRef backend_llvm_function_call_compile(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx)
{
    // for now just deal with the built-in print() function
    static const struct RFstring s = RF_STRING_STATIC_INIT("print");
    if (rf_string_equal(ast_fncall_name(n), &s)) {
        struct ast_node *args = ast_fncall_args(n);
        //for now should be only one arg so ...
        LLVMValueRef compiled_args = backend_llvm_expression_compile(args, ctx);
        LLVMValueRef call_args[] = { compiled_args };
        LLVMBuildCall(ctx->builder, LLVMGetNamedFunction(ctx->mod, "print"),
                      call_args,
                      1, "");
    }
    return NULL;
}

LLVMValueRef backend_llvm_expression_compile_vardecl(struct ast_node *n,
                                                     struct llvm_traversal_ctx *ctx)
{
    // all vardelcs should have had stack size allocated during block symbol iteration
    struct symbol_table_record *rec;
    struct ast_node *left = ast_typedesc_left((ast_vardecl_desc_get(n)));
    AST_NODE_ASSERT_TYPE(left, AST_IDENTIFIER);

    rec = symbol_table_lookup_record(ctx->current_st,
                                     ast_identifier_str(left), NULL);
    RF_ASSERT(rec->backend_handle, "No LLVMValue was determined for a vardecl");
    return rec->backend_handle;
}

LLVMValueRef backend_llvm_expression_compile_string_literal(struct ast_node *n,
                                                            struct llvm_traversal_ctx *ctx)
{
    // all unique string literals should have been declared as global strings
    uint32_t hash;
    const struct RFstring *s = ast_string_literal_get_str(n);
    if (!string_table_add_or_get_str(ctx->rir->string_literals_table, s, &hash)) {
        RF_ERROR("Unable to retrieve string literal from table during LLVM compile");
        return NULL;
    }
    RFS_buffer_push();
    const char *cstr = rf_string_cstr_from_buff(RFS_("gstr_%u", hash));
    LLVMValueRef global_str = LLVMGetNamedGlobal(ctx->mod, cstr);
    RFS_buffer_pop();
    return global_str;
}

LLVMValueRef backend_llvm_expression_compile_identifier(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx)
{
    struct symbol_table_record *rec;
    const struct RFstring *s = ast_identifier_str(n);
    rec = symbol_table_lookup_record(ctx->current_st, s, NULL);
    RF_ASSERT(rec->backend_handle, "No LLVMValue was determined for "
              "identifier \""RF_STR_PF_FMT"\"", RF_STR_PF_ARG(s));
    return rec->backend_handle;
}

LLVMValueRef backend_llvm_expression_compile(struct ast_node *n,
                                             struct llvm_traversal_ctx *ctx)
{
    uint64_t val;
    LLVMValueRef llvm_val;
    switch(n->type) {
    case AST_BINARY_OPERATOR:
        return backend_llvm_expression_compile_bop(n, ctx);
    case AST_RETURN_STATEMENT:
        llvm_val = backend_llvm_expression_compile(ast_returnstmt_expr_get(n), ctx);
        return LLVMBuildRet(ctx->builder, llvm_val);
    case AST_FUNCTION_CALL:
        return backend_llvm_function_call_compile(n, ctx);
    case AST_CONSTANT_NUMBER:
        if (!ast_constantnum_get_integer(n, &val)) {
            RF_ERROR("Failed to convert a constant num node to number for LLVM");
        }
        // TODO: This is not using rir_types ... also maybe get rid of ctx->current_value?
        //       if we are going to be returning it anyway?
        ctx->current_value = LLVMConstInt(LLVMInt32Type(), val, 0);
        return ctx->current_value;
    case AST_STRING_LITERAL:
        return backend_llvm_expression_compile_string_literal(n, ctx);
    case AST_IDENTIFIER:
        return backend_llvm_expression_compile_identifier(n, ctx);
    case AST_VARIABLE_DECLARATION:
        return backend_llvm_expression_compile_vardecl(n, ctx);
    default:
        RF_ASSERT(false, "Illegal node type at LLVM code generation");
        break;
    }
    return NULL;
}

static void backend_llvm_expression(struct ast_node *n,
                                    struct llvm_traversal_ctx *ctx)
{
    ctx->current_value = NULL;
    backend_llvm_expression_compile(n, ctx);
    ctx->current_value = NULL;
}

static void llvm_symbols_iterate_cb(struct symbol_table_record *rec,
                                    struct llvm_traversal_ctx *ctx)
{
    char *name;
    // for each symbol, allocate an LLVM variable in the stack with alloca
    struct rir_type *type = symbol_table_record_rir_type(rec);
    RFS_buffer_push();
    name = rf_string_cstr_from_buff(symbol_table_record_id(rec));
    // note: this simply creates the stack space but does not allocate it
    LLVMTypeRef llvm_type = backend_llvm_type(type, ctx);
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, llvm_type, name);
    symbol_table_record_set_backend_handle(rec, allocation);
    RFS_buffer_pop();
}

static void backend_llvm_basic_block(struct rir_basic_block *block,
                                     struct llvm_traversal_ctx *ctx)
{
    struct ast_node *expr;
    struct symbol_table *prev = ctx->current_st;
    ctx->current_st = block->symbols;

    symbol_table_iterate(block->symbols, (htable_iter_cb)llvm_symbols_iterate_cb, ctx);
    rf_ilist_for_each(&block->lh, expr, ln_for_rir_blocks) {
        backend_llvm_expression(expr, ctx);
    }

    ctx->current_st = prev;
}

static LLVMValueRef backend_llvm_function(struct rir_function *fn,
                                          struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef llvm_fn;
    char *fn_name;
    char *param_name;
    RFS_buffer_push();
    fn_name = rf_string_cstr_from_buff(&fn->name);
    // evaluating types here since you are not guaranteed order of execution of
    // a function's arguments and this does have sideffects we read from
    // llvm_traversal_ctx_get_param_count()
    LLVMTypeRef * types = backend_llvm_fn_arg_types(fn->arg_type, ctx);
    llvm_fn = LLVMAddFunction(ctx->mod, fn_name,
                              LLVMFunctionType(backend_llvm_type(fn->ret_type, ctx),
                                               types,
                                               llvm_traversal_ctx_get_param_count(ctx),
                                               false)); // never variadic for now
    RFS_buffer_pop();

    // now handle function body
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(llvm_fn, "entry");
    LLVMPositionBuilderAtEnd(ctx->builder, entry);
    // place function's argument in the stack
    unsigned int i = 0;
    LLVMValueRef allocation;
    const struct RFstring *param_name_str;
    struct symbol_table_record *rec;

    if (fn->arg_type->category != ELEMENTARY_RIR_TYPE_NIL) {
        for (i = 0; i <LLVMCountParams(llvm_fn); ++i) {

            // for each argument of the function allocate an LLVM variable
            // in the stack with alloca
            param_name_str = rir_type_get_nth_name(fn->arg_type, i);
            RFS_buffer_push();
            param_name = rf_string_cstr_from_buff(param_name_str);
            allocation = LLVMBuildAlloca(ctx->builder,
                                         backend_llvm_type(rir_type_get_nth_type(fn->arg_type, i), ctx),
                                         param_name);
            RFS_buffer_pop();
            // and assign to it the argument value
            LLVMBuildStore(ctx->builder, LLVMGetParam(llvm_fn, i) ,allocation);
            // also note the alloca in the symbol table
            rec = symbol_table_lookup_record(fn->symbols, param_name_str, NULL);
            RF_ASSERT_OR_CRITICAL(rec, "Symbol table of rir_function did "
                                  "not contain expected parameter");
            symbol_table_record_set_backend_handle(rec, allocation);
        }
    }

    // now handle function entry block
    backend_llvm_basic_block(fn->entry, ctx);
    return llvm_fn;
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

static void backend_llvm_create_const_strings(const struct string_table_record *rec,
                                              struct llvm_traversal_ctx *ctx)
{
    unsigned int length = rf_string_length_bytes(&rec->string);
    char *gstr_name;
    char *strbuff_name;

    RFS_buffer_push();
    strbuff_name = rf_string_cstr_from_buff(RFS_("strbuff_%u", rec->hash));
    LLVMValueRef global_stringbuff = backend_llvm_add_global_strbuff(rf_string_cstr_from_buff(&rec->string),
                                                                     length,
                                                                     strbuff_name,
                                                                     ctx);

    LLVMValueRef indices_0 [] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), 0, 0) };
    LLVMValueRef gep_to_string_buff = LLVMConstInBoundsGEP(global_stringbuff, indices_0, 2);
    LLVMValueRef string_struct_layout[] = { LLVMConstInt(LLVMInt32Type(), length, 0), gep_to_string_buff };
    LLVMValueRef string_decl = LLVMConstNamedStruct(LLVMGetTypeByName(ctx->mod, "string"), string_struct_layout, 2);

    gstr_name = rf_string_cstr_from_buff(RFS_("gstr_%u", rec->hash));
    LLVMValueRef global_val = LLVMAddGlobal(ctx->mod, LLVMGetTypeByName(ctx->mod, "string"), gstr_name);
    RFS_buffer_pop();
    LLVMSetInitializer(global_val, string_decl);
}

static bool backend_llvm_create_global_functions(struct llvm_traversal_ctx *ctx)
{
    /* -- add print() -- */

    // print() uses clib's printf so we need a declaration for it
    LLVMTypeRef printf_args[] = { LLVMPointerType(LLVMInt8Type(), 0) };
    LLVMAddFunction(ctx->mod, "printf",
                              LLVMFunctionType(LLVMInt32Type(),
                                               printf_args,
                                               1,
                                               true));

    // add print()
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
    return true;
}

static bool backend_llvm_create_globals(struct llvm_traversal_ctx *ctx)
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
                         (string_table_cb)backend_llvm_create_const_strings, ctx);
    if (!backend_llvm_create_global_functions(ctx)) {
        RF_ERROR("Could not create global functions");
        return false;
    }
    return true;
}

struct LLVMOpaqueModule *backend_llvm_create_module(struct rir_module *mod,
                                                    struct llvm_traversal_ctx *ctx)
{
    struct rir_function *fn;
    const char *mod_name;
    RFS_buffer_push();
    mod_name = rf_string_cstr_from_buff(&mod->name);
    ctx->mod = LLVMModuleCreateWithName(mod_name);

    if (!backend_llvm_create_globals(ctx)) {
        RF_ERROR("Failed to create global context for LLVM");
        LLVMDisposeModule(ctx->mod);
        ctx->mod = NULL;
        goto end;
    }

    // for each function of the module create code
    rf_ilist_for_each(&mod->functions, fn, ln_for_module) {
        backend_llvm_function(fn, ctx);
    }

    if (ctx->args->print_backend_debug) {
        // would be much better if we could call llvm::Module::getModuleIdentifier()
        // but that seems to not be exposed in the C-Api
        mod_name = rf_string_cstr_from_buff(&mod->name);
        backend_llvm_mod_debug(ctx->mod, mod_name);
    }

end:
    RFS_buffer_pop();
    return ctx->mod;
}
