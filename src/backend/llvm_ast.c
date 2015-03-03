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
#include <types/type.h>

#include <analyzer/string_table.h>

#include <ir/elements.h>
#include <ir/rir_type.h>
#include <ir/rir.h>

#include <backend/llvm.h>
#include "llvm_utils.h"
#include "llvm_globals.h"

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

    case ELEMENTARY_RIR_TYPE_FLOAT_32:
        return LLVMFloatType();
    case ELEMENTARY_RIR_TYPE_FLOAT_64:
        return LLVMDoubleType();

    case ELEMENTARY_RIR_TYPE_STRING:
        return LLVMGetTypeByName(ctx->mod, "string");

    case ELEMENTARY_RIR_TYPE_NIL:
        return LLVMVoidType();
    default:
        RF_ASSERT(false,
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
    char *name;
    LLVMTypeRef ret = NULL;
    if (rir_type_is_elementary(type)) {
        ret = backend_llvm_elementary_to_type(type->category, ctx);
    } else if (type->category == COMPOSITE_RIR_DEFINED) {
        RFS_buffer_push();
        name = rf_string_cstr_from_buff(type->name);
        ret = LLVMGetTypeByName(ctx->mod, name);
        RFS_buffer_pop();
    } else {
        RF_ASSERT(false, "Not yet implemented type");
    }

    RF_ASSERT(ret, "The above functions should never fail");
    return ret;
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

// return an array of member types of a defined type
static LLVMTypeRef *backend_llvm_defined_member_types(struct rir_type *type,
                                                      struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(type->category == COMPOSITE_RIR_DEFINED, "Called with non defined type");
    struct rir_type **subtype;
    RF_ASSERT(darray_size(type->subtypes) == 1,
              "A defined type should always have 1 direct subtype");
    struct rir_type *defined_content_type = darray_item(type->subtypes, 0);
    LLVMTypeRef llvm_type;
    llvm_traversal_ctx_reset_params(ctx);
    if (darray_size(defined_content_type->subtypes) == 0) {
        llvm_type = backend_llvm_type(type, ctx);
        if (llvm_type != LLVMVoidType()) {
            llvm_traversal_ctx_add_param(ctx, llvm_type);
        }
    } else {
        darray_foreach(subtype, defined_content_type->subtypes) {
            llvm_type = backend_llvm_type(*subtype, ctx);
            if (llvm_type != LLVMVoidType()) {
                llvm_traversal_ctx_add_param(ctx, llvm_type);
            }
        }
    }
    return llvm_traversal_ctx_get_params(ctx);
}



void backend_llvm_assign_defined_types(LLVMValueRef left,
                                       LLVMValueRef right,
                                       struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef left_cast = LLVMBuildBitCast(ctx->builder, left,
                                              LLVMPointerType(LLVMInt8Type(), 0), "");
    LLVMValueRef right_cast = LLVMBuildBitCast(ctx->builder, right,
                                               LLVMPointerType(LLVMInt8Type(), 0), "");
    LLVMValueRef llvm_memcpy = LLVMGetNamedFunction(ctx->mod, "llvm.memcpy.p0i8.p0i8.i64");
    /* RF_ASSERT(LLVMIsAMemCpyInst(llvm_memcpy), "Should have retrieved llvm memcpy instance"); */

    LLVMValueRef call_args[] = { left_cast, right_cast,
                                 LLVMConstInt(LLVMInt64Type(), 8, 0),
                                 LLVMConstInt(LLVMInt32Type(), 0, 0),
                                 LLVMConstInt(LLVMInt1Type(), 0, 0) };
    LLVMBuildCall(ctx->builder, llvm_memcpy, call_args, 5, "");
}


static LLVMValueRef backend_llvm_compile_assign(struct ast_node *n,
                                                LLVMValueRef left,
                                                LLVMValueRef right,
                                                struct llvm_traversal_ctx *ctx)
{
    if (type_is_specific_elementary(n->expression_type, ELEMENTARY_TYPE_STRING)) {
        AST_NODE_ASSERT_TYPE(ast_binaryop_right(n), AST_STRING_LITERAL);
        LLVMValueRef length;
        LLVMValueRef string_data;
        backend_llvm_load_from_string(right, &length, &string_data, ctx);
        backend_llvm_assign_to_string(left, length, string_data, ctx);
    } else if (type_category_equals(n->expression_type, TYPE_CATEGORY_DEFINED)) {
        backend_llvm_assign_defined_types(left, right, ctx);
    } else {
        RF_ASSERT(false, "Not yet implemented");
    }

    // hm what should compiling an assignment return?
    return NULL;
}

static LLVMValueRef backend_llvm_compile_member_access(struct ast_node *n,
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

static LLVMValueRef backend_llvm_expression_compile_bop(struct ast_node *n,
                                                        struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_BINARY_OPERATOR);

    if (ast_binaryop_op(n) == BINARYOP_MEMBER_ACCESS) {
        return backend_llvm_compile_member_access(n, ctx);
    }

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
        return backend_llvm_compile_assign(n, left, right, ctx);
    default:
        RF_ASSERT(false, "Illegal binary operation type at LLVM code generation");
        break;
    }
    return NULL;
}


struct args_to_value_cb_ctx {
    unsigned int index;
    unsigned int offset;
    struct llvm_traversal_ctx *llvm_ctx;
    LLVMValueRef alloca;
    LLVMTypeRef *params;
};

static void args_to_value_cb_ctx_init(struct args_to_value_cb_ctx *ctx,
                                      struct llvm_traversal_ctx *llvm_ctx,
                                      LLVMValueRef alloca,
                                      LLVMTypeRef *params)
{
    ctx->index = 0;
    ctx->offset = 0;
    ctx->llvm_ctx = llvm_ctx;
    ctx->alloca = alloca;
    ctx->params = params;
}

static void backend_llvm_store(LLVMValueRef val, LLVMValueRef ptr,
                               struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef val_type = LLVMTypeOf(val);
    LLVMTypeRef ptr_element_type = LLVMGetElementType(LLVMTypeOf(ptr));

    if (val_type != ptr_element_type) {
        // we have to do typecasts
        if (val_type == LLVMDoubleType()) {
            val = LLVMBuildFPCast(ctx->builder, val, ptr_element_type, "");
        } else if (val_type == LLVMInt8Type() || val_type == LLVMInt16Type() ||
                   val_type == LLVMInt32Type() || val_type == LLVMInt64Type()) {
            val = LLVMBuildIntCast(ctx->builder, val, ptr_element_type, "");
        } else {
            backend_llvm_type_debug(val_type, "val_type");
            backend_llvm_type_debug(ptr_element_type, "ptr_element_type");
            RF_ASSERT(false, "Unimplemented casts?");
        }
    }
    LLVMBuildStore(ctx->builder, val, ptr);
}

static bool args_to_value_cb(struct ast_node *n, struct args_to_value_cb_ctx *ctx)
{
    LLVMValueRef arg_value = backend_llvm_expression_compile(n, ctx->llvm_ctx);
    LLVMValueRef indices[] = { LLVMConstInt(LLVMInt32Type(), 0, 0), LLVMConstInt(LLVMInt32Type(), ctx->offset, 0) };
    LLVMValueRef gep = LLVMBuildGEP(ctx->llvm_ctx->builder, ctx->alloca, indices, 2, "");

    backend_llvm_store(arg_value, gep, ctx->llvm_ctx);
    ctx->offset += 1;
    ctx->index++;
    return true;
}

static LLVMValueRef backend_llvm_ctor_args_to_type(struct ast_node *fn_call,
                                                   const struct RFstring *type_name,
                                                   struct llvm_traversal_ctx *ctx)
{
    struct args_to_value_cb_ctx cb_ctx;
    char *name;

    // alloca enough space in the stack for the type created by the constructor
    RFS_buffer_push();
    name = rf_string_cstr_from_buff(type_name);
    LLVMTypeRef llvm_type = LLVMGetTypeByName(ctx->mod, name);
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, llvm_type, "");
    RFS_buffer_pop();

    struct rir_type *defined_type = rir_types_list_get_defined(&ctx->rir->rir_types_list, type_name);
    LLVMTypeRef *params = backend_llvm_defined_member_types(defined_type, ctx);

    args_to_value_cb_ctx_init(&cb_ctx, ctx, allocation, params);
    ast_fncall_for_each_arg(fn_call, (fncall_args_cb)args_to_value_cb, &cb_ctx);

    return allocation;
}

LLVMValueRef backend_llvm_function_call_compile(struct ast_node *n,
                                                struct llvm_traversal_ctx *ctx)
{
    // for now just deal with the built-in print() function
    static const struct RFstring s = RF_STRING_STATIC_INIT("print");
    const struct RFstring *fn_name = ast_fncall_name(n);
    const struct type *fn_type;
    struct ast_node *args = ast_fncall_args(n);
    if (rf_string_equal(fn_name, &s)) {
        // for now, print only accepts 1 argument
        LLVMValueRef compiled_args = backend_llvm_expression_compile(args, ctx);
        LLVMValueRef call_args[] = { compiled_args };
        LLVMBuildCall(ctx->builder, LLVMGetNamedFunction(ctx->mod, "print"),
                      call_args,
                      1, "");
    } else {
        fn_type = type_lookup_identifier_string(fn_name, ctx->current_st);
        if (type_is_function(fn_type)) {
            // TODO
            RF_ASSERT(false, "Not yet implemented");
        } else {
            RF_ASSERT(fn_type->category == TYPE_CATEGORY_DEFINED,
                      "At this point the only possible type should be defined type");

            return backend_llvm_ctor_args_to_type(n, fn_name, ctx);
        }
    }

    // if function returns void. TODO: Maybe handle better
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

void backend_llvm_compile_typedecl(const struct RFstring *name,
                                   struct rir_type *type,
                                   struct llvm_traversal_ctx *ctx)
{
    char *name_cstr;
    RFS_buffer_push();
    name_cstr = rf_string_cstr_from_buff(name);
    LLVMTypeRef llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(),
                                                 name_cstr);
    RFS_buffer_pop();

    if (!type) {
        type = rir_types_list_get_defined(&ctx->rir->rir_types_list, name);
    }
    LLVMTypeRef *members = backend_llvm_defined_member_types(type, ctx);
    LLVMStructSetBody(llvm_type, members, llvm_traversal_ctx_get_param_count(ctx), true);
}

LLVMValueRef backend_llvm_expression_compile(struct ast_node *n,
                                             struct llvm_traversal_ctx *ctx)
{
    uint64_t int_val;
    double float_val;
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
        if (ast_constantnum_get_type(n) == CONSTANT_NUMBER_INTEGER) {
            if (!ast_constantnum_get_integer(n, &int_val)) {
                RF_ERROR("Failed to convert a constant num node to integer number for LLVM");
            }
            // TODO: This is not using rir_types ... also maybe get rid of ctx->current_value?
            //       if we are going to be returning it anyway?
            ctx->current_value = LLVMConstInt(LLVMInt32Type(), int_val, 0);
        } else {
            RF_ASSERT(ast_constantnum_get_type(n) == CONSTANT_NUMBER_FLOAT,
                      "Only other choice for constant number type here should be float");
            if (!ast_constantnum_get_float(n, &float_val)) {
                RF_ERROR("Failed to convert a constant num node to float number for LLVM");
            }
            ctx->current_value = LLVMConstReal(LLVMDoubleType(), float_val);
        }
        return ctx->current_value;
    case AST_STRING_LITERAL:
        return backend_llvm_expression_compile_string_literal(n, ctx);
    case AST_IDENTIFIER:
        return backend_llvm_expression_compile_identifier(n, ctx);
    case AST_VARIABLE_DECLARATION:
        return backend_llvm_expression_compile_vardecl(n, ctx);
    case AST_TYPE_DECLARATION:
        backend_llvm_compile_typedecl(ast_typedecl_name_str(n), NULL, ctx);
        break;
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
    struct rir_expression *rir_expr;
    struct symbol_table *prev = ctx->current_st;
    ctx->current_st = block->symbols;

    symbol_table_iterate(block->symbols, (htable_iter_cb)llvm_symbols_iterate_cb, ctx);
    rf_ilist_for_each(&block->expressions, rir_expr, ln) {
        backend_llvm_expression(rir_expr->expr, ctx);
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





struct LLVMOpaqueModule *backend_llvm_create_module(struct rir_module *mod,
                                                    struct llvm_traversal_ctx *ctx)
{
    struct rir_function *fn;
    const char *mod_name;
    RFS_buffer_push();
    mod_name = rf_string_cstr_from_buff(&mod->name);
    ctx->mod = LLVMModuleCreateWithName(mod_name);
    ctx->target_data = LLVMCreateTargetData(LLVMGetDataLayout(ctx->mod));

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
