#ifndef LFR_IR_RIR
#define LFR_IR_RIR

#include <RFintrusive_list.h>

struct module;
struct rir_types_list;

struct rir {
    //! A list of all rir types of the file
    struct rir_types_list *rir_types_list;
    //! List of functions
    struct RFilist_head functions;
};

struct rir_ctx {
    struct rir *rir;
    struct rir_fndecl *current_fn;
    struct rir_block *current_block;
};

struct rir *rir_create(struct module *m);
void rir_destroy(struct rir* r);
#endif
