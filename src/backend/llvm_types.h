#ifndef LFR_BACKEND_LLVM_TYPES_H
#define LFR_BACKEND_LLVM_TYPES_H

#include <rflib/datastructs/htable.h>
#include <types/type_decls.h>

struct RFstring;
struct rir_type;
struct rir_type;
struct type;
struct llvm_traversal_ctx;
struct LLVMOpaqueType;
struct rir_typedef;
struct rir_type_arr;;

struct rir_types_map {
    //! Hash for the map
    struct htable table;
};

bool rir_types_map_init(struct rir_types_map *m);
void rir_types_map_deinit(struct rir_types_map *m);

/**
 * Adds a type to the map. We rely on the fact that during the analysis stage
 * we have made sure there are no duplicates, since this simply hashes the pointer
 *
 * @param m            The map to add to
 * @param rtype        The rir type to add
 * @param lstruct      The LLVM struct type to map to the @a rir_type
 * @return             true in success and false in failure
 */
bool rir_types_map_add(struct rir_types_map *m,
                       struct rir_type *rtype,
                       struct LLVMOpaqueType *lstruct);

/**
 * Retrieve an LLVM struct from the map
 *
 * @param m            The map to retrieve from
 * @param rtype        The rir type whose struct to retrieve
 * @return             The LLVM struct type that corresponds to @a rtype
 *                     or NULL if there is no map entry for the type
 */
struct LLVMOpaqueType *rir_types_map_get(struct rir_types_map *m,
                                         struct rir_type *rtype);

/**
 * Compiles a typedef
 *
 * @param def         The typedef to compile
 * @param ctx         The llvm traversal context
 * @return            The LLVM type for the compiled struct or NULL in error
 */
struct LLVMOpaqueType *bllvm_compile_typedef(const struct rir_typedef *def,
                                             struct llvm_traversal_ctx *ctx);

/**
 * Compile a type as internal type declaration
 * @param type        The type to compile
 * @param ctx         The llvm traversal context
 * @return            The LLVM type for the compiled struct or NULL in error
 */
struct LLVMOpaqueType *bllvm_compile_internal_typedecl(const struct type *type,
                                                       struct llvm_traversal_ctx *ctx);


/**
 * Given a rir type array create the equivalent llvm type array
 *
 * @warning Uses the context's parameters array.
 * Call llvm_traversal_ctx_reset_params(ctx) to clear right after using.
 *
 * @param typearr        The rir arguments array
 * @param ctx         The llvm traversal context
 */
struct LLVMOpaqueType **bllvm_rir_to_llvm_types(const struct rir_type_arr *typearr,
                                                struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueType *bllvm_type_from_rir_type(const struct rir_type *type,
                                                 struct llvm_traversal_ctx *ctx);

struct LLVMOpaqueType *bllvm_elementary_to_type(enum elementary_type etype,
                                                struct llvm_traversal_ctx *ctx);

/**
 * Given an LLVMType check if it's any int type
 */
bool bllvm_type_is_int(const struct LLVMOpaqueType *type);
/**
 * Given an LLVMType check if it's a float/double type
 */
bool bllvm_type_is_floating(const struct LLVMOpaqueType *type);
/**
 * Given an LLVMType check if it's elementary plain old data type
 */
bool bllvm_type_is_elementary(const struct LLVMOpaqueType *type);
#endif
