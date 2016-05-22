#include "llvm_types.h"
#include "llvm_ast.h"
#include "llvm_utils.h"

#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Transforms/Scalar.h>

#include <rfbase/utils/hash.h>
#include <rfbase/string/common.h>
#include <rfbase/string/conversion.h>

#include <analyzer/symbol_table.h>
#include <types/type.h>

#include <ir/rir_typedef.h>
#include <ir/rir_object.h>

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

static LLVMTypeRef bllvm_create_struct(const struct RFstring *name)
{
    const char *name_cstr;
    RFS_PUSH();
    name_cstr = rf_string_cstr_from_buff_or_die(name);
    LLVMTypeRef llvm_type = LLVMStructCreateNamed(LLVMGetGlobalContext(),
                                                  name_cstr);
    RFS_POP();
    return llvm_type;
}

LLVMTypeRef bllvm_compile_typedef(const struct rir_typedef *def,
                                  struct llvm_traversal_ctx *ctx)
{
    llvm_traversal_ctx_reset_params(ctx);
    // else it's the same thing but just need to add an extra index for the union
    LLVMTypeRef llvm_type = bllvm_create_struct(&def->name);
    bllvm_rir_to_llvm_types(&def->argument_types, ctx);
    if (def->is_union) { // add the member selector in the beginning
        llvm_traversal_ctx_prepend_param(ctx, LLVMInt32Type());
    }
    LLVMStructSetBody(llvm_type, llvm_traversal_ctx_get_params(ctx), llvm_traversal_ctx_get_param_count(ctx), true);
    llvm_traversal_ctx_reset_params(ctx);
    return llvm_type;
}

LLVMTypeRef bllvm_elementary_to_type(enum elementary_type etype,
                                     struct llvm_traversal_ctx *ctx)
{
    switch(etype) {
        // LLVM does not differentiate between signed and unsigned
    case ELEMENTARY_TYPE_INT_8:
    case ELEMENTARY_TYPE_UINT_8:
        return LLVMInt8Type();
    case ELEMENTARY_TYPE_INT_16:
    case ELEMENTARY_TYPE_UINT_16:
        return LLVMInt16Type();
    case ELEMENTARY_TYPE_INT_32:
    case ELEMENTARY_TYPE_UINT_32:
        return LLVMInt32Type();
    case ELEMENTARY_TYPE_INT:
    case ELEMENTARY_TYPE_UINT:
    case ELEMENTARY_TYPE_INT_64:
    case ELEMENTARY_TYPE_UINT_64:
        return LLVMInt64Type();

    case ELEMENTARY_TYPE_FLOAT_32:
        return LLVMFloatType();
    case ELEMENTARY_TYPE_FLOAT_64:
        return LLVMDoubleType();

    case ELEMENTARY_TYPE_STRING:
        return LLVMGetTypeByName(ctx->llvm_mod, "string");

    case ELEMENTARY_TYPE_BOOL:
        return LLVMInt1Type();
    case ELEMENTARY_TYPE_NIL:
        return LLVMVoidType();

    default:
        RF_CRITICAL_FAIL(
            "Unsupported elementary type \""RFS_PF"\" during LLVM conversion",
            RFS_PA(type_elementary_get_str(etype))
        );
        break;
    }
    return NULL;
}

LLVMTypeRef bllvm_type_from_rir_type(
    const struct rir_type *type,
    struct llvm_traversal_ctx *ctx)
{
    LLVMTypeRef ret;
    switch (type->category) {
    case RIR_TYPE_ELEMENTARY:
        ret = bllvm_elementary_to_type(type->etype, ctx);
        break;
    case RIR_TYPE_COMPOSITE:
        RFS_PUSH();
        ret = LLVMGetTypeByName(
            ctx->llvm_mod,
            rf_string_cstr_from_buff_or_die(&type->tdef->name)
        );
        RF_ASSERT(ret, "Type should have already been declared in LLVM");
        RFS_POP();
        break;
    case RIR_TYPE_ARRAY:
    {
        RF_ASSERT(type->array.size > 0, "Only fixed size array types supported for now");
        const struct rir_type *member_type = rir_type_array_membertype(type);
        ret = LLVMArrayType(
            bllvm_type_from_rir_type(member_type, ctx),
            type->array.size
        );
        break;
    }
    default:
        RF_CRITICAL_FAIL("Unexpected rir type encountered");
        return NULL;
    }

    // if it's a pointer to the type
    if (type->is_pointer) {
        ret = LLVMPointerType(ret, 0);
    }
    return ret;
}

LLVMTypeRef *bllvm_rir_to_llvm_types(const struct rir_type_arr *arr,
                                     struct llvm_traversal_ctx *ctx)
{
    llvm_traversal_ctx_reset_params(ctx);
    struct rir_type **argtype;
    LLVMTypeRef llvm_type;
    darray_foreach(argtype, *arr) {
        llvm_type = bllvm_type_from_rir_type(*argtype, ctx);
        llvm_traversal_ctx_add_param(ctx, llvm_type);
    }
    return llvm_traversal_ctx_get_params(ctx);
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
