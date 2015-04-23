#ifndef LFR_BACKEND_LLVM_TYPES_H
#define LFR_BACKEND_LLVM_TYPES_H

#include <Data_Structures/htable.h>

struct RFstring;
struct rir_type;
struct llvm_traversal_ctx;
struct LLVMOpaqueType;

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
 * Compile a type declaration
 *
 * @param name        Provide the name of the type to create
 * @param type        [optional] Can provide the rir_type to declare here.
 *                    IF it's NULL then the type is searched for in the rir types list
 * @param ctx         The llvm traversal context
 * @return            The LLVM type for the compiled struct or NULL in error
 */
struct LLVMOpaqueType *backend_llvm_compile_typedecl(const struct RFstring *name,
                                                     struct rir_type *type,
                                                     struct llvm_traversal_ctx *ctx);

/**
 * Given a rir type return all its subtypes in an LLVMType Array.
 *
 * This is supposed to work only on non-sum type types
 *
 * @param type         The rir sum type whose operands to add to the array
 * @param ctx          The llvm traversal context
 * @return             An point to an array of LLVM Types held by the context
 */
struct LLVMOpaqueType **backend_llvm_type_to_subtype_array(const struct rir_type *type,
                                                           struct llvm_traversal_ctx *ctx);

/**
 * Given a simple rir type get its defined member types as an array of LLVMTypeRefs
 *
 * @param type        The rir type whose members to get. Must not be a sum type
 * @param ctx         The llvm traversal context
 */
struct LLVMOpaqueType **backend_llvm_simple_member_types(struct rir_type *type,
                                                         struct llvm_traversal_ctx *ctx);

#endif
