#ifndef LFR_IR_RIR
#define LFR_IR_RIR

#include <RFintrusive_list.h>

struct module;

struct rir {
    //! List of functions
    struct RFilist_head functions;
};

struct rir *rir_create(struct module *m);
void rir_destroy(struct rir* r);
#endif
