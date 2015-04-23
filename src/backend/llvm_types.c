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

static inline const void *rir_types_map_key_(const void *elem)
{
	return elem;
}
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

static LLVMTypeRef backend_llvm_compile_simple_typedecl(const struct RFstring *name,
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
    LLVMTypeRef *members = backend_llvm_simple_defined_member_types(type, ctx);
    LLVMStructSetBody(llvm_type, members, llvm_traversal_ctx_get_param_count(ctx), true);
    return llvm_type;
}

LLVMTypeRef backend_llvm_compile_typedecl(const struct RFstring *name,
                                          struct rir_type *type,
                                          struct llvm_traversal_ctx *ctx)
{
    // temporary just to test if the string name even matters
    static unsigned int count = 0;
    if (!type) {
        type = rir_types_list_get_defined(&ctx->rir->rir_types_list, name);
    }
    if (!rir_type_is_sumtype(type)) {
        return backend_llvm_compile_simple_typedecl(name, type, ctx);
    }
    // TODO: if it's a sum type we have to add the selector variable to the
    // body and also create structures of all the possible subtypes and
    // provide the biggest one as the body (+ the selector)
    struct rir_type **subtype;
    struct rir_type *contents = darray_item(type->subtypes, 0);
    unsigned long long max_storage_size = 0;
    darray_foreach(subtype, contents->subtypes) {

        LLVMTypeRef subtype_llvm_type = rir_types_map_get(&ctx->types_map, *subtype);
        if (!subtype_llvm_type) {
            subtype_llvm_type = backend_llvm_compile_typedecl(
                RFS_OR_DIE("internal_struct%u", count),
                *subtype,
                ctx
            );
            ++count;
            if (!subtype_llvm_type) {
                return NULL;
            }
            // put it in the map
            rir_types_map_add(&ctx->types_map, *subtype, subtype_llvm_type);
        }
        unsigned long long this_size = LLVMStoreSizeOfType(ctx->target_data, subtype_llvm_type);
        if (max_storage_size > this_size) {
            max_storage_size = this_size;
        }
    }
    // make an array to fit the biggest sum
    LLVMTypeRef body = LLVMArrayType(LLVMInt8Type(), max_storage_size);
    // TODO: This is identical to start of backend_llvm_compile_simple_typedecl()
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

struct LLVMOpaqueType **backend_llvm_type_to_subtype_array(const struct rir_type *type,
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
    return llvm_traversal_ctx_get_params(ctx);
}


LLVMTypeRef *backend_llvm_simple_defined_member_types(struct rir_type *type,
                                                      struct llvm_traversal_ctx *ctx)
{
    RF_ASSERT(type->category == COMPOSITE_RIR_DEFINED, "Called with non defined type");
    RF_ASSERT(darray_size(type->subtypes) == 1,
              "A defined type should always have 1 direct subtype");
    RF_ASSERT(!rir_type_is_sumtype(type), "Called with defined type with sum type contents");
    return backend_llvm_type_to_subtype_array(darray_item(type->subtypes, 0), ctx);
}
