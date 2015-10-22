#ifndef LFR_TESTSUPPORT_RIR_H
#define LFR_TESTSUPPORT_RIR_H

#include "../testsupport_front.h"
#include "../analyzer/testsupport_analyzer.h"
#include <ir/rir.h>
#include <Definitions/inline.h>

struct rir_testdriver {
    struct front_testdriver *front_driver;
    struct analyzer_testdriver *analyzer_driver;
    //! RIR modules created by the user, specifying the desired RIR state at each test
    struct {darray(struct rir*);} target_rirs;

    struct rir *curr_module;
    struct rir_fndef *curr_fn;
    struct rir_block *curr_block;
};
struct rir_testdriver *get_rir_testdriver();

void setup_rir_tests();
void setup_rir_tests_no_stdlib();
void setup_rir_tests_no_source();
void setup_rir_tests_with_filelog();
void teardown_rir_tests();

#define ck_assert_createrir_ok()                                        \
    do {                                                                \
        ck_assert_typecheck_ok();                                       \
        if (!compiler_create_rir()) {                                   \
            testsupport_show_front_errors("Creating the RIR failed");   \
        }                                                               \
    } while(0)

/* -- Functions that facilitate the specification of a target RIR -- */

i_INLINE_DECL void testsupport_rir_set_curr_module(struct rir *r)
{
    get_rir_testdriver()->curr_module = r;
}

i_INLINE_DECL void testsupport_rir_set_curr_fn(struct rir_fndef *fn)
{
    get_rir_testdriver()->curr_fn = fn;
}

i_INLINE_DECL void testsupport_rir_set_curr_block(struct rir_block *block)
{
    get_rir_testdriver()->curr_block = block;
}


struct rir *testsupport_rir_add_module();
// TODO ..
/* struct rir *testsupport_rir_add_fn(const struct RFstring *name, ); */



#endif
