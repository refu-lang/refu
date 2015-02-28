#ifndef LFR_BACKEND_LLVM_UTILS_H
#define LFR_BACKEND_LLVM_UTILS_H

struct LLVMOpaqueModule;
struct LLVMOpaqueValue;
struct LLVMOpaqueType;

/* -- Some debugging functions to help in sticky situations -- */
void backend_llvm_val_debug(struct LLVMOpaqueValue *v, const char *val_name);
void backend_llvm_type_debug(struct LLVMOpaqueType *t, const char *type_name);
void backend_llvm_mod_debug(struct LLVMOpaqueModule *m, const char *mod_name);


#endif