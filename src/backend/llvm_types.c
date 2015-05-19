#include "llvm_types.h"
#include "llvm_ast.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <Utils/hash.h>
#include <String/rf_str_common.h>
#include <String/rf_str_conversion.h>

#include <ir/rir.h>
#include <ir/rir_types_list.h>
#include <ir/rir_type.h>

static inline size_t rir_types_map_hash_(const void *elem, void *priv)
{
	return hash_pointer(elem, 0);
}
static inline bool rir_types_map_eq_(const void *e1, void *e2)
{
	return e1 == e2;
}

bool rir_types_map_init(struct rir_types_map *m)
{
    htable_init(&m->table, rir_types_map_hash_, NULL);
    return true;
}

void rir_types_map_deinit(struct rir_types_map *m)
{
    htable_clear(&m->table);
}

bool rir_types_map_add(struct rir_types_map *m,
                       struct rir_type *rtype,
                       struct LLVMOpaqueType *lstruct)
{
    return htable_add(&m->table, hash_pointer(rtype, 0), lstruct);
}

struct LLVMOpaqueType *rir_types_map_get(struct rir_types_map *m,
                                         struct rir_type *rtype)
{
    return htable_get(&m->table, hash_pointer(rtype, 0), rir_types_map_eq_, rtype);
}

static LLVMTypeRef bllvm_compile_simple_typedecl(const struct RFstring *name,
                                                        struct rir_type *type,
                                                        struct llvm_traversal_ctx *ctx)
{
    if (!type) {
        type = rir_types_list_get_defined(&ctx->rir->rir_types_list, name);
    }
    RF_ASSERT(!rir_type_is_sumtype(type), "Should not be called with sumtype");

    char *name_cstr;
    RFS_PUSH();
    name_cstr = rf_string_cstr_from_buff_or_die(name);
    LLVMTypeRef llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(),
                                                  name_cstr);
    RFS_POP();
    LLVMTypeRef *members = bllvm_simple_member_types(type, ctx);
    LLVMStructSetBody(llvm_type, members, llvm_traversal_ctx_get_param_count(ctx), true);
    llvm_traversal_ctx_reset_params(ctx);
    return llvm_type;
}

LLVMTypeRef bllvm_compile_internal_typedecl(const struct type *type,
                                            struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef ret;
    RFS_PUSH();
    ret = bllvm_compile_typedecl(
        type_get_unique_type_str(type),
        type,
        ctx
    );
    RFS_POP();
    return ret;
}

LLVMTypeRef bllvm_compile_typedecl(const struct RFstring *name,
                                   const struct type *type,
                                   struct llvm_traversal_ctx *ctx)
{
    if (!type) {
        type = symbol_table_lookup_defined_type(&ctx->current_st, name);
    }

    if (!rir_type_is_sumtype(type)) {
        return bllvm_compile_simple_typedecl(name, type, ctx);
    }
    // if it's a sum type we have to add the selector variable to the
    // body and also create structures of all the possible subtypes and
    // provide the biggest one as the body (+ the selector)
    struct rir_type **subtype;
    struct rir_type *contents = type->category == COMPOSITE_RIR_DEFINED
        ? darray_item(type->subtypes, 0) // contents of a defined type
        : type;                          // anonymous type is itself the contents
    unsigned long long max_storage_size = 0;
    darray_foreach(subtype, contents->subtypes) {

        LLVMTypeRef subtype_llvm_type = rir_types_map_get(&ctx->types_map, *subtype);
        if (!subtype_llvm_type) {
            subtype_llvm_type = bllvm_compile_internal_typedecl(*subtype, ctx);
            if (!subtype_llvm_type) {
                return NULL;
            }
            // put it in the map
            rir_types_map_add(&ctx->types_map, *subtype, subtype_llvm_type);
        }
        unsigned long long this_size = LLVMStoreSizeOfType(ctx->target_data, subtype_llvm_type);
        if (this_size > max_storage_size) {
            max_storage_size = this_size;
        }
    }
    RF_ASSERT(max_storage_size != 0, "Loop did not run?");
    // make an array to fit the biggest sum
    LLVMTypeRef body = LLVMArrayType(LLVMInt8Type(), max_storage_size);
    // TODO: This is identical to start of bllvm_compile_simple_typedecl()
    //       abstract properly
    char *name_cstr;
    RFS_PUSH();
    name_cstr = rf_string_cstr_from_buff_or_die(name);
    LLVMTypeRef llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(),
                                                  name_cstr);
    RFS_POP();
    // the struct needs enough space to fit the biggest sum operand + int32 for selector
    LLVMTypeRef llvm_struct_contents[] = { body, LLVMInt32Type() };
    LLVMStructSetBody(llvm_type, llvm_struct_contents, 2, true);
    return llvm_type;
}

struct LLVMOpaqueType **bllvm_type_to_subtype_array(const struct rir_type *type,
                                                    struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(type->category != COMPOSITE_RIR_DEFINED ||
              type->category != COMPOSITE_SUM_RIR_TYPE ||
              type->category != COMPOSITE_IMPLICATION_RIR_TYPE,
              "Called with illegal rir type");
    LLVMTypeRef llvm_type;
    struct rir_type **subtype;
    llvm_traversal_ctx_reset_params(ctx);
    if (darray_size(type->subtypes) == 0) {
        llvm_type = bllvm_type_from_rir(type, ctx);
        if (llvm_type != LLVMVoidType()) {
            llvm_traversal_ctx_add_param(ctx, llvm_type);
        }
    } else {
        darray_foreach(subtype, type->subtypes) {
            llvm_type = bllvm_type_from_rir(*subtype, ctx);
            if (llvm_type != LLVMVoidType()) {
                llvm_traversal_ctx_add_param(ctx, llvm_type);
            }
        }
    }
    return llvm_traversal_ctx_get_params(ctx);
}


LLVMTypeRef *bllvm_simple_member_types(struct rir_type *type,
                                       struct llvm_traversal_ctx *ctx)
{
    struct rir_type *actual_type = type;
    if (type->category == COMPOSITE_RIR_DEFINED) {
        RF_ASSERT(darray_size(type->subtypes) == 1,
                  "A defined type should always have 1 direct subtype");
        actual_type = darray_item(type->subtypes, 0);
    }
    RF_ASSERT(!rir_type_is_sumtype(actual_type), "Called with sum type contents");
    return bllvm_type_to_subtype_array(actual_type, ctx);
}

bool bllvm_type_is_int(const struct LLVMOpaqueType *type)
{
    return type == LLVMInt8Type() || type == LLVMInt16Type() ||
        type == LLVMInt32Type() || type == LLVMInt64Type();
}

bool bllvm_type_is_floating(const struct LLVMOpaqueType *type)
{
    return type == LLVMDoubleType() || type == LLVMFloatType();
}

bool bllvm_type_is_elementary(const struct LLVMOpaqueType *type)
{
    return bllvm_type_is_int(type) || bllvm_type_is_floating(type);
}
