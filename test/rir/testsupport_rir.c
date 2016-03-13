#include "testsupport_rir.h"

#include "../testsupport.h"
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <ir/rir_convert.h>
#include <ir/rir_call.h>
#include <ir/rir_binaryop.h>
#include <ir/parser/rirparser.h>


static struct rir_testdriver i_rir_test_driver_;

struct rir_testdriver *get_rir_testdriver()
{
    return &i_rir_test_driver_;
}

static bool rir_testdriver_init(struct rir_testdriver *d,
                                struct front_testdriver *front_driver,
                                struct analyzer_testdriver *analyzer_driver)
{
    d->front_driver = front_driver;
    d->analyzer_driver = analyzer_driver;
    darray_init(d->target_rirs);
    return true;
}

static void rir_testdriver_deinit(struct rir_testdriver *d)
{
    // free the target rirs
    struct rir **r;
    darray_foreach(r, d->target_rirs) {
        rir_destroy(*r);
    }
    darray_free(d->target_rirs);
}

void setup_rir_tests()
{
    setup_analyzer_tests();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}

void setup_rir_tests_no_stdlib()
{
    setup_analyzer_tests_no_stdlib();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void setup_rir_tests_with_filelog()
{
    setup_analyzer_tests_with_filelog();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
}
void setup_rir_tests_no_source()
{
    setup_analyzer_tests_no_stdlib();
    ck_assert_msg(rir_testdriver_init(&i_rir_test_driver_, get_front_testdriver(), get_analyzer_testdriver()),
                  "Failed to initialize the rir test driver");
    front_testdriver_new_ast_main_source(rf_string_empty_get());
    testsupport_analyzer_prepare();
}

void teardown_rir_tests()
{
    rir_testdriver_deinit(&i_rir_test_driver_);
    teardown_analyzer_tests();
}

/* -- Functions that facilitate the specification of a target RIR -- */
struct rir *testsupport_rir_add_module()
{
    struct rir_testdriver *tdr = get_rir_testdriver();
    struct rir *r = rir_create();
    if (!r) {
        return NULL;
    }
    darray_append(tdr->target_rirs, r);
    rir_pctx_init(testsupport_rir_pctx(), r);
    return r;
}

struct rir_typedef *testsupport_rir_add_typedef_impl(
    const char *namecstr,
    bool is_union,
    const char *filename,
    unsigned int line
)
{
    RFS_PUSH();
    struct rir_object *obj = rir_typedef_create_obj(
        testsupport_rir_curr_module(),
        NULL, // curr_fn
        RFS("%s", namecstr),
        is_union,
        NULL // args will be added later. This is only happening in testing
    );
    RFS_POP();
    ck_assert_msg(
        obj,
        CK_ABORT_LESTR "Could not create a typedef rir object",
        filename,
        line
    );
    return &obj->tdef;
}

struct rir_fndef *testsupport_rir_add_fndef_impl(
    char *name,
    struct rir_type **given_args,
    size_t given_args_size,
    struct rir_type *return_type
)
{
    struct rir_fndef *def;
    RF_MALLOC(def, sizeof(*def), return false);
    RF_STRUCT_ZERO(def);
    // prepare arguments for the actual fndecl_init() function
    const struct RFstring sname = RF_STRING_SHALLOW_INIT_CSTR(name);

    struct rir_type_arr args;
    if (given_args) {
        struct rir_type **newarr = malloc(given_args_size);
        memcpy(newarr, given_args, given_args_size);
        darray_raw_copy(args, newarr, given_args_size / sizeof(struct rir_type*));
    } else {
        darray_init(args); // no args - empty array
    }

    if (!return_type) {
        return_type = (struct rir_type*)rir_type_elem_get(ELEMENTARY_TYPE_NIL, false);
    }

    // let's use same initialization rir parsing uses
    if (!rir_fndecl_init(
            &def->decl,
            &sname,
            &args,
            return_type,
            false, //not foreign
            RIRPOS_PARSE,
            &get_rir_testdriver()->pctx)) {
        ck_abort_msg("Failed to initialize an fndecl");
        return NULL;
    }

    if (!rir_fndef_init_no_decl(def, RIRPOS_PARSE, testsupport_rir_pctx())) {
        ck_abort_msg("Failed to initialize an fndef");
        return NULL;
    }

    rf_ilist_add_tail(&testsupport_rir_curr_module()->functions, &def->decl.ln);
    rir_data_curr_fn(&get_rir_testdriver()->pctx) = def;
    return def;
}

struct rir_expression *testsupport_rir_add_call_impl(
    char *retid,
    char *name,
    bool is_foreign,
    struct rir_value **given_args,
    size_t given_args_size
)
{
    const struct RFstring sretid = RF_STRING_SHALLOW_INIT_CSTR(retid);
    rir_pctx_set_id(testsupport_rir_pctx(), &sretid);
    const struct RFstring sname = RF_STRING_SHALLOW_INIT_CSTR(name);
    struct rir_value **newarr = malloc(given_args_size);
    memcpy(newarr, given_args, given_args_size);
    struct value_arr args;
    darray_raw_copy(args, newarr, given_args_size / sizeof(struct rir_value*));
    struct rir_object *obj = rir_call_create_obj(
        &sname,
        &args,
        is_foreign,
        RIRPOS_PARSE,
                    &get_rir_testdriver()->pctx
    );
    rir_pctx_reset_id(testsupport_rir_pctx());
    return obj ? &obj->expr : NULL;
}


struct rir_block *testsupport_rir_add_block(char *name)
{
    const struct RFstring bname = RF_STRING_SHALLOW_INIT_CSTR(name);
    struct rir_object *obj = rir_block_create_obj(
        &bname, RIRPOS_PARSE, testsupport_rir_pctx()
    );
    rir_fndef_add_block(testsupport_rir_curr_fn(), &obj->block);
    rir_data_curr_block(&get_rir_testdriver()->pctx) = &obj->block;
    return &obj->block;
}

struct rir_expression *testsupport_rir_add_convert(
    char *name,
    struct rir_value *v,
    struct rir_type *type
)
{
    const struct RFstring cname = RF_STRING_SHALLOW_INIT_CSTR(name);
    rir_pctx_set_id(testsupport_rir_pctx(), &cname);
    struct rir_object *o = rir_convert_create_obj(
        v,
        type,
        RIRPOS_PARSE,
        testsupport_rir_pctx()
    );
    rir_pctx_reset_id(testsupport_rir_pctx());
    return o ? &o->expr : NULL;
}

struct rir_expression *testsupport_rir_add_write(
    struct rir_value *memory,
    struct rir_value *val
)
{
    return rir_write_create(memory, val, RIRPOS_PARSE, testsupport_rir_pctx());
}

struct rir_expression *testsupport_rir_add_read(
    char *name,
    struct rir_value *memory
)
{
    const struct RFstring rname = RF_STRING_SHALLOW_INIT_CSTR(name);
    rir_pctx_set_id(testsupport_rir_pctx(), &rname);
    struct rir_expression *ret = rir_read_create(
        memory,
        RIRPOS_PARSE,
        testsupport_rir_pctx()
    );
    rir_pctx_reset_id(testsupport_rir_pctx());
    return ret;
}

struct rir_expression *testsupport_rir_add_binaryop(
    enum rir_expression_type type,
    char *id,
    const struct rir_value *a,
    const struct rir_value *b
)
{
    const struct RFstring sid = RF_STRING_SHALLOW_INIT_CSTR(id);
    rir_pctx_set_id(testsupport_rir_pctx(), &sid);
    struct rir_expression *ret = rir_binaryop_create_nonast(
        type, a, b, RIRPOS_PARSE, testsupport_rir_pctx()
    );
    rir_pctx_reset_id(testsupport_rir_pctx());
    return ret;
}

struct rir_value *testsupport_rir_value(char *name)
{
    const struct RFstring vname = RF_STRING_SHALLOW_INIT_CSTR(name);
    struct rir_object *obj = rir_map_getobj(testsupport_rir_pctx().common, &vname);
    ck_assert_msg(
        obj, "Could not find value of variable \""RFS_PF"\" in the current map.",
        RFS_PA(&vname)
    );
    return rir_object_value(obj);
}
