#include "llvm_functions.h"
#include "llvm_matchexpr.h"
#include "llvm_ast.h"

#include <llvm-c/Core.h>

#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>

#include <module.h>
#include <analyzer/symbol_table.h>
#include <analyzer/analyzer.h>
#include <ast/function.h>
#include <ast/type.h>
#include <types/type.h>
#include <types/type_function.h>
#include <ir/rir_types_list.h>
#include <ir/rir_type.h>
#include <ir/rir.h>

#include "llvm_ast.h"
#include "llvm_utils.h"

struct ctor_args_to_value_cb_ctx {
    unsigned int index;
    unsigned int offset;
    struct llvm_traversal_ctx *llvm_ctx;
    LLVMValueRef alloca;
    LLVMTypeRef *params;
};

static void ctor_args_to_value_cb_ctx_init(struct ctor_args_to_value_cb_ctx *ctx,
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

static bool ctor_args_to_value_cb(struct ast_node *n, struct ctor_args_to_value_cb_ctx *ctx)
{
    LLVMValueRef arg_value = bllvm_compile_expression(n, ctx->llvm_ctx, 0);
    LLVMValueRef gep = bllvm_gep_to_struct(ctx->alloca, ctx->offset, ctx->llvm_ctx);
    bllvm_store(arg_value, gep, ctx->llvm_ctx);
    ctx->offset += 1;
    ctx->index++;
    return true;
}


static LLVMValueRef bllvm_assign_params_to_defined_type(struct ast_node *fn_call,
                                                        LLVMTypeRef internal_subtype,
                                                        LLVMTypeRef *params,
                                                        struct llvm_traversal_ctx *ctx)
{
    struct ctor_args_to_value_cb_ctx cb_ctx;
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, internal_subtype, "");
    ctor_args_to_value_cb_ctx_init(&cb_ctx, ctx, allocation, params);
    ast_fncall_for_each_arg(fn_call, (fncall_args_cb)ctor_args_to_value_cb, &cb_ctx);
    return allocation;
}

static LLVMValueRef bllvm_simple_ctor_args_to_type(struct ast_node *fn_call,
                                                   const struct RFstring *type_name,
                                                   struct llvm_traversal_ctx *ctx)
{
    char *name;
    // alloca enough space in the stack for the type created by the constructor
    RFS_PUSH();
    name = rf_string_cstr_from_buff_or_die(type_name);
    LLVMTypeRef llvm_type = LLVMGetTypeByName(ctx->llvm_mod, name);
    RFS_POP();

    LLVMTypeRef *params;
    struct rir_type *defined_type = rir_types_list_get_defined(ctx->mod->rir->rir_types_list, type_name);
    RF_ASSERT(!rir_type_is_sumtype(defined_type), "Called with sum type");
    params = bllvm_simple_member_types(defined_type, ctx);
    return bllvm_assign_params_to_defined_type(fn_call, llvm_type, params, ctx);
}

static LLVMValueRef bllvm_sum_fncall_args_to_type(struct ast_node *fn_call,
                                                  struct rir_type *sum_type,
                                                  const struct RFstring *type_name,
                                                  struct llvm_traversal_ctx *ctx)
{
    const struct type *params_type = ast_fncall_params_type(fn_call);
    const struct rir_type *params_rtype = type_get_rir_or_die(params_type);
    // find out the index of the sum operand type in the defined type
    int child_index = rir_type_childof_type(params_rtype, sum_type);
    RF_ASSERT(child_index != -1, "fncall arg should be a child of original functions sum type");
    // get the LLVM struct type of the sum operand
    RFS_PUSH();
    LLVMTypeRef llvm_sum_type = LLVMGetTypeByName(
        ctx->llvm_mod,
        rf_string_cstr_from_buff_or_die(type_get_unique_type_str(params_type, false))
    );
    RFS_POP();
    RF_ASSERT(llvm_sum_type, "Internal struct was not created for sum operand");
    // populate the sum operand's internal struct contents
    LLVMTypeRef *params = bllvm_type_to_subtype_array(params_rtype, ctx);
    LLVMValueRef populated_sum_type = bllvm_assign_params_to_defined_type(fn_call, llvm_sum_type, params, ctx);
    // now create the full struct and assign the contents and the selector
    RFS_PUSH();
    LLVMTypeRef llvm_type = LLVMGetTypeByName(ctx->llvm_mod,
                                              rf_string_cstr_from_buff_or_die(type_name));
    LLVMValueRef allocation = LLVMBuildAlloca(ctx->builder, llvm_type, "");
    RFS_POP();
    LLVMValueRef gep_to_main_contents = bllvm_gep_to_struct(allocation, 0, ctx);
    bllvm_memcpy(populated_sum_type, gep_to_main_contents, ctx);
    // here also set the second value of the struct (the alloca) which should be the selector
    LLVMValueRef gep_to_selector = bllvm_gep_to_struct(allocation, 1, ctx);
    bllvm_store(LLVMConstInt(LLVMInt32Type(), child_index, 0), gep_to_selector, ctx);    
    return allocation;
}

static LLVMValueRef bllvm_sum_ctor_args_to_type(struct ast_node *fn_call,
                                                const struct RFstring *type_name,
                                                struct llvm_traversal_ctx *ctx)
{
    struct rir_type *defined_type = rir_types_list_get_defined(ctx->mod->rir->rir_types_list, type_name);
    RF_ASSERT(rir_type_is_sumtype(defined_type), "Called with non sum type");
    return bllvm_sum_fncall_args_to_type(
        fn_call,
        darray_item(defined_type->subtypes, 0), // contents of a defined type is first child
        type_name, // name of the type is the constructor name
        ctx
    );
    
}

static LLVMValueRef bllvm_ctor_args_to_type(struct ast_node *fn_call,
                                            const struct RFstring *type_name,
                                            struct llvm_traversal_ctx *ctx)
{
    struct rir_type *defined_type = rir_types_list_get_defined(ctx->mod->rir->rir_types_list, type_name);
    if (rir_type_is_sumtype(defined_type)) {
        return bllvm_sum_ctor_args_to_type(fn_call, type_name, ctx);
    }
    return bllvm_simple_ctor_args_to_type(fn_call, type_name, ctx);
}

static bool fncall_args_to_value_cb(struct ast_node *n, struct llvm_traversal_ctx *ctx)
{
    LLVMValueRef arg_value = bllvm_compile_expression(n, ctx, RFLLVM_OPTION_IDENTIFIER_VALUE);
    /* LLVMValueRef arg_value = bllvm_compile_expression(n, ctx, 0); */
    llvm_traversal_ctx_add_value(ctx, arg_value);
    return true;
}

LLVMValueRef bllvm_compile_functioncall(struct ast_node *n,
                                        struct llvm_traversal_ctx *ctx)
{
    const struct RFstring *fn_name = ast_fncall_name(n);
    const struct type *fn_type;
    struct ast_node *args = ast_fncall_args(n);
    fn_type = type_lookup_identifier_string(fn_name, ctx->current_st);
    if (type_is_function(fn_type)) {
        RFS_PUSH();
        char *fn_name_cstr = rf_string_cstr_from_buff_or_die(fn_name);
        LLVMValueRef llvm_fn = LLVMGetNamedFunction(ctx->llvm_mod, fn_name_cstr);
        RFS_POP();
        LLVMTypeRef llvm_fn_type = bllvm_function_type(llvm_fn);

        const struct type *fn_args_type = type_function_get_argtype(fn_type);
        llvm_traversal_ctx_reset_values(ctx);
        if (type_is_sumop(fn_args_type)) {
            // if the function takes an anonymous sum type argument we need special handling
            llvm_traversal_ctx_add_value(
                ctx,
                bllvm_sum_fncall_args_to_type(
                    n,
                    rir_types_list_get_type(&ctx->mod->rir->rir_types_list->lh, fn_args_type, NULL),
                    type_get_unique_type_str(fn_args_type, false),
                    ctx)
            );
        } else {
            ast_fncall_for_each_arg(n, (fncall_args_cb)fncall_args_to_value_cb, ctx);
            RF_ASSERT(
                LLVMCountParamTypes(llvm_fn_type) == llvm_traversal_ctx_get_values_count(ctx),
                "Function \""RF_STR_PF_FMT"()\" receiving unexpected number of "
                "arguments in backend code generation", RF_STR_PF_ARG(fn_name));
        }

        return LLVMBuildCall(ctx->builder,
                             llvm_fn,
                             llvm_traversal_ctx_get_values(ctx),
                             llvm_traversal_ctx_get_values_count(ctx),
                             "");
    } else if (fn_type->category == TYPE_CATEGORY_DEFINED) {
        return bllvm_ctor_args_to_type(n, fn_name, ctx);
    } else {
        RF_ASSERT(type_is_explicitly_convertable_elementary(fn_type),
                  "At this point the only possible call should be explicit cast");
        return bllvm_compile_explicit_cast(fn_type, args, ctx);
    }

    RF_CRITICAL_FAIL("should never get here");
    return NULL;
}

// compile function args and return an array of LLVMTypeRef
static LLVMTypeRef *bllvm_compile_fn_arg_types(const struct type *type,
                                               struct llvm_traversal_ctx *ctx)
{
    if (!type_is_sumtype(type)) {
        bllvm_type_to_subtype_array(type_get_rir_or_die(type), ctx);
    } else {
        // sum type argument function, compile an internal typedecl and add as single parameter
        llvm_traversal_ctx_reset_params(ctx);
        LLVMTypeRef internal_struct_type = bllvm_compile_internal_typedecl(type, ctx);        
        llvm_traversal_ctx_add_param(
            ctx,
            LLVMPointerType(internal_struct_type, 0) // At least for now all structs passed by reference
        );
    }
    return darray_size(ctx->params) == 0 ? NULL : llvm_traversal_ctx_get_params(ctx);
}

/**
 * Get the cstring name of a parameter of a type
 *
 * @warning Needs to be enclosed in RFS_PUSH()/RFS_POP()
 * If the type is a sumtype then just create a dummy name same as the type name
 */
static inline const struct RFstring *bllvm_param_name_str(const struct type *type,
                                                          struct llvm_traversal_ctx *ctx,
                                                          unsigned int i)
{
    return type_is_sumtype(type)
        ? type_get_unique_value_str(type, false)
        : type_get_nth_name_or_die(type, i);
}

LLVMValueRef bllvm_compile_function(struct ast_node *fn,
                                    struct llvm_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(fn, AST_FUNCTION_IMPLEMENTATION);
    struct ast_node *fn_decl = ast_fnimpl_fndecl_get(fn);
    const struct type *fn_args = ast_node_get_type_or_nil(
        ast_fndecl_args_get(fn_decl),
        AST_TYPERETR_AS_LEAF
    );
    const struct type *fn_ret = ast_node_get_type_or_nil(
        ast_fndecl_return_get(fn_decl),
        AST_TYPERETR_AS_LEAF
    );
    char *fn_name;
    char *param_name;
    unsigned args_num = ast_fndecl_argsnum_get(fn_decl);
    RFS_PUSH();
    fn_name = rf_string_cstr_from_buff_or_die(ast_fndecl_name_str(fn_decl));
    // evaluating types here since you are not guaranteed order of execution of
    // a function's arguments and this does have sideffects we read from
    // llvm_traversal_ctx_get_param_count()
    LLVMTypeRef *types = bllvm_compile_fn_arg_types(fn_args, ctx);
    ctx->current_function = LLVMAddFunction(
        ctx->llvm_mod,
        fn_name,
        LLVMFunctionType(
            bllvm_type_from_type(fn_ret, ctx),
            types,
            llvm_traversal_ctx_get_param_count(ctx),
            false // never variadic for now
        )
    ); 
    RFS_POP();

    // now handle function body
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(ctx->current_function, "entry");
    bllvm_enter_block(ctx, entry);
    // place function's argument in the stack
    unsigned int i = 0;
    LLVMValueRef allocation;
    struct symbol_table_record *rec;
    const struct RFstring *param_name_str;

    if (args_num != 0) {
        for (i = 0; i < args_num; ++i) {
            LLVMTypeRef alloca_type;

            // for each argument of the function allocate an LLVM variable
            // in the stack with alloca
            RFS_PUSH();
            param_name_str = bllvm_param_name_str(fn_args, ctx, i);
            param_name =  rf_string_cstr_from_buff_or_die(param_name_str);

            // if it's not a function call on an anonymous sum type
            if (!type_is_sumtype(fn_args)) {
                alloca_type = bllvm_type_from_type(type_get_nth_type_or_die(fn_args, i), ctx);
                allocation = LLVMBuildAlloca(
                    ctx->builder,
                    alloca_type,
                    param_name
                );
                // and assign to it the argument value
                bllvm_store(LLVMGetParam(ctx->current_function, i), allocation, ctx);
                // also note the alloca in the symbol table
                rec = symbol_table_lookup_record(
                    ast_fnimpl_symbol_table_get(fn),
                    param_name_str,
                    NULL
                );
                RFS_POP();
                RF_ASSERT(rec,
                          "Symbol table of rir_function did "
                          "not contain expected parameter");
                symbol_table_record_set_backend_handle(rec, allocation);
            }
        }
    }

    // if the function's got a return value alloc it here
    ctx->current_function_return = NULL;
    if (!type_is_specific_elementary(fn_ret, ELEMENTARY_TYPE_NIL)) {
        ctx->current_function_return = LLVMBuildAlloca(
            ctx->builder,
            bllvm_type_from_type(fn_ret, ctx),
            "function_return_value"
        );
    }

    // this block should always stay at the end of the function
    LLVMBasicBlockRef function_end = LLVMAppendBasicBlock(ctx->current_function, "function_end");
    // now compile all parts of the function
    struct ast_node *body = ast_fnimpl_body_get(fn);
    if (body->type == AST_BLOCK) {
        bllvm_compile_block(ast_fnimpl_body_get(fn), ctx);
    } else {
        // it's a match expression for a body
        bllvm_compile_matchexpr(body, ctx);
    }

    // finally build the function return. Jump from whichever the second last block
    // was to the return block and return
    bllvm_add_br(function_end, ctx);
    bllvm_enter_block(ctx, function_end);
    if (type_is_specific_elementary(fn_ret, ELEMENTARY_TYPE_NIL)) {
        LLVMBuildRetVoid(ctx->builder);
    } else { // if we got a return value
        // I suppose in some case no load would be needed. Need to abstract these
        // differentiations somehow
        LLVMValueRef ret = LLVMBuildLoad(ctx->builder, ctx->current_function_return, "");
        LLVMBuildRet(ctx->builder, ret);
    }
    return ctx->current_function;
}

LLVMTypeRef bllvm_function_type(LLVMValueRef fn)
{
    return LLVMGetElementType(LLVMTypeOf(fn));
}
