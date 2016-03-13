#ifndef LFR_TESTSUPPORT_RIR_H
#define LFR_TESTSUPPORT_RIR_H

#include "../testsupport_front.h"
#include "../analyzer/testsupport_analyzer.h"
#include <ir/rir.h>
#include <ir/rir_global.h>
#include <ir/rir_constant.h>
#include <ir/rir_block.h>
#include <ir/rir_expression.h>
#include <ir/rir_typedef.h>
#include <ir/parser/rirparser.h>
#include <Definitions/inline.h>

struct rir_testdriver {
    struct front_testdriver *front_driver;
    struct analyzer_testdriver *analyzer_driver;
    //! RIR modules created by the user, specifying the desired RIR state at each test
    struct {darray(struct rir*);} target_rirs;
    //! A parser context holding the current rir module, function, block
    struct rir_pctx pctx;
};
struct rir_testdriver *get_rir_testdriver();
#define testsupport_rir_pctx()                  \
    &get_rir_testdriver()->pctx
#define testsupport_rir_curr_module()           \
    rir_data_rir(testsupport_rir_pctx())
#define testsupport_rir_curr_fn()               \
    rir_data_curr_fn(testsupport_rir_pctx())
#define testsupport_rir_curr_block()            \
    rir_data_curr_block(testsupport_rir_pctx())

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

// A hacky convenience macro to get to the rir stage and get the rir object
// in order to test it for other purposes
#define ck_create_get_rir(result_, index_)                              \
    do {                                                                \
        ck_assert_createrir_ok();                                       \
        result_ = darray_item(compiler_instance_get()->modules, index_)->rir; \
            if (!result_) {                                             \
                ck_abort_msg(                                           \
                    "Could not get the %u RIR object from the compiler", index_ \
                );                                                      \
            }                                                           \
    } while (0)

/* -- Functions that facilitate the specification of a target RIR -- */

struct rir *testsupport_rir_add_module();
#define testsupport_rir_add_gstring(i_val_)                             \
    do {                                                                \
        static const struct RFstring temps_ = RF_STRING_STATIC_INIT(i_val_); \
        RFS_PUSH();                                                     \
        const struct RFstring *namestr = RFS("$gstr_%u", rf_hash_str_stable(&temps_, 0)); \
        rir_global_create_string(                                       \
            rir_type_elem_get(ELEMENTARY_TYPE_STRING, false),           \
            namestr,                                                    \
            &temps_,                                                    \
            testsupport_rir_curr_module()                               \
        );                                                              \
    } while(0)

#define testsupport_rir_add_typedef(name_, is_union_)                   \
    testsupport_rir_add_typedef_impl(name_, is_union_, __FILE__, __LINE__)
struct rir_typedef *testsupport_rir_add_typedef_impl(
    const char *name,
    bool is_union,
    const char *filename,
    unsigned int line
);

#define testsupport_rir_add_fndef(name_, args_, return_)                \
    testsupport_rir_add_fndef_impl(name_, args_, sizeof(args_), return_)
struct rir_fndef *testsupport_rir_add_fndef_impl(
    char *name,
    struct rir_type **given_args,
    size_t given_args_size,
    struct rir_type *return_type
);

#define testsupport_rir_add_call(retid_, name_, foreign_, args_)        \
    testsupport_rir_add_call_impl(retid_, name_, foreign_, args_, sizeof(args_))
struct rir_expression *testsupport_rir_add_call_impl(
    char *retid,
    char *name,
    bool is_foreign,
    struct rir_value **given_args,
    size_t given_args_size
);

struct rir_block *testsupport_rir_add_block(char *name);

#define testsupport_rir_block_add_cmd(block_, type_, ...)               \
    do {                                                                \
        struct rir_expression *expr = testsupport_rir_add_##type_(__VA_ARGS__); \
        ck_assert_msg(expr, "Failed to create a \""#type_"\" expression."); \
        rir_block_add_expr(block_, expr);                               \
    } while (0)

struct rir_expression *testsupport_rir_add_convert(
    char *name,
    struct rir_value *v,
    struct rir_type *type
);

struct rir_expression *testsupport_rir_add_write(
    struct rir_value *memory,
    struct rir_value *val
);

struct rir_expression *testsupport_rir_add_read(
    char *name,
    struct rir_value *memory
);

#define testsupport_rir_block_add_bop(block_, type_, id_, vala_, valb_) \
    do {                                                                \
        struct rir_expression *expr = testsupport_rir_add_binaryop(     \
            type_,                                                      \
            id_,                                                        \
            vala_,                                                      \
            valb_                                                       \
        );                                                              \
        ck_assert_msg(expr, "Failed to create a rir bop");              \
        rir_block_add_expr(block_, expr);                               \
    } while (0)

struct rir_expression *testsupport_rir_add_binaryop(
    enum rir_expression_type type,
    char *id,
    const struct rir_value *a,
    const struct rir_value *b
);


#define testsupport_rir_intvalue(value_)                                \
    rir_constantval_create_fromint64(value_, testsupport_rir_curr_module())
struct rir_value *testsupport_rir_value(char *name);

// needs to be enclosed in RFS_PUSH()/RFS_POP()
#define testsupport_rir_etype(type_, is_ptr_)                           \
    (struct rir_type*)rir_type_elem_get_from_string(RFS("%s", type_), is_ptr_)

#define testsupport_rir_ctype(type_, is_ptr_)                           \
    {.category=RIR_TYPE_COMPOSITE, .is_pointer=is_ptr_, .tdef=type_}

// make a copy of the static arguments array and put the copy in the darray.
// We are doing this because the darray takes ownership of the copy and will
// free it later. If we just did a raw copy, there would be an attempt to free
// a static array.
#define testsupport_rir_typedef_add_arguments(tdef_, args_) \
    do {                                                    \
        struct rir_type **newarr = malloc(sizeof(args_));   \
        memcpy(newarr, args_, sizeof(args_));               \
        darray_raw_copy(                                    \
            (tdef_)->argument_types,                        \
            newarr,                                         \
            sizeof(args_)/sizeof(struct rir_type*)          \
        );                                                  \
    } while (0)

#define testsupport_rir_typearr(tdef_, elemcstr_)                       \
    do {                                                                \
        RFS_PUSH();                                                     \
        darray_append(                                                  \
            (tdef_)->argument_types,                                    \
            (struct rir_type*)rir_type_elem_get_from_string(RFS("%s", elemcstr_), false) \
        );                                                              \
        RFS_POP();                                                      \
    } while(0)


#define ck_assert_parserir(gotrir_)                             \
    ck_assert_parserir_impl(                                    \
        __FILE__,                                               \
        __LINE__,                                               \
        gotrir_                                                 \
    )

bool ck_assert_parserir_impl(
    const char *filename,
    unsigned int line,
    struct rir *got_rir
);

#endif
