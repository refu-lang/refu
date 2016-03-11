#include "testsupport_rir.h"

#include "../testsupport.h"
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>
#include <ir/rir_convert.h>
#include <ast/constants.h>
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
    struct rir_type **newarr = malloc(given_args_size);
    memcpy(newarr, given_args, given_args_size);
    struct rir_type_arr args;
    darray_raw_copy(args, newarr, given_args_size / sizeof(struct rir_type*));
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

struct rir_value *testsupport_rir_value(char *name)
{
    const struct RFstring vname = RF_STRING_SHALLOW_INIT_CSTR(name);
    struct rir_object *obj = rir_map_getobj(testsupport_rir_pctx().common, &vname);
    ck_assert_msg(
        obj, "Could not find value of variable \""RF_STR_PF_FMT"\" in"
        " the current map.",
        RF_STR_PF_ARG(&vname)
    );
    return rir_object_value(obj);
}

/* ---- RIR comparison functions follow ---- */

static bool ckr_compare_type(
    const struct rir_type *got,
    const struct rir_type *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
);

static bool ckr_compare_expression(
    struct rir_expression *got,
    struct rir_expression *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
);

static bool testrir_compare_global(
    struct rir_global *got,
    struct rir_global *expect,
    const char *filename,
    unsigned int line
)
{
    const struct RFstring *got_name = rir_global_name(got);
    const struct RFstring *expect_name = rir_global_name(expect);
    if (!rf_string_equal(got_name, expect_name)) {
        ck_abort_at(
            filename,
            line,
            "Failure at the RIR global comparison",
            "Expected a global with name "
            "\""RF_STR_PF_FMT"\" but got one named \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(expect_name),
            RF_STR_PF_ARG(got_name)
        );
        return false;
    }

    struct rir_type *got_type = rir_global_type(got);
    struct rir_type *expect_type = rir_global_type(expect);
    if (!rir_type_identical(got_type, expect_type)) {
        ck_abort_at(
            filename,
            line,
            "Failure at the RIR global comparison",
            "Expected a global with type "
            "\""RF_STR_PF_FMT"\" but got one with type \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rir_type_string(expect_type)),
            RF_STR_PF_ARG(rir_type_string(got_type))
        );
        return false;
    }
    return true;
}

static bool ckr_compare_constant(
    const struct ast_constant *got,
    const struct ast_constant *expect,
    const char *file,
    unsigned int line
)
{
    if (got->type != expect->type) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR constant comparison",
            "Expected 'type' to be \""RF_STR_PF_FMT"\" but "
            "it is \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(ast_constant_type_string(expect)),
            RF_STR_PF_ARG(ast_constant_type_string(got))
        );
        return false;
    }

    switch(got->type) {
    case CONSTANT_NUMBER_FLOAT:
        if (got->value.floating != expect->value.floating) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR constant comparison",
                "Expected %f but got %f",
                got->value.floating,
                expect->value.floating
            );
            return false;
        }
        break;
    case CONSTANT_NUMBER_INTEGER:
        if (got->value.integer != expect->value.integer) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR constant comparison",
                "Expected %d but got %d",
                got->value.integer,
                expect->value.integer
            );
            return false;
        }
        break;
    case CONSTANT_BOOLEAN:
        if (got->value.boolean != expect->value.boolean) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR constant comparison",
                "Expected '%s' but got '%s'",
                FMT_BOOL(got->value.boolean),
                FMT_BOOL(expect->value.boolean)
            );
            return false;
        }
        break;
    }
    return true;
}

static bool ckr_compare_value(
    const struct rir_value *got,
    const struct rir_value *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
)
{
    if (got->category != expect->category) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RF_STR_PF_FMT". Expected 'category' to be \""RF_STR_PF_FMT"\" but "
            "it is \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(intro),
            RF_STR_PF_ARG(rir_value_type_string(expect)),
            RF_STR_PF_ARG(rir_value_type_string(got))
        );
        return false;
    }

    if (!rf_string_equal(&got->id, &expect->id)) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RF_STR_PF_FMT". Expected 'id' to be \""RF_STR_PF_FMT"\" but "
            "it is \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(intro),
            RF_STR_PF_ARG(rir_value_string(expect)),
            RF_STR_PF_ARG(rir_value_string(got))
        );
        return false;
    }

    if (got->type != expect->type) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RF_STR_PF_FMT". Value type existence mismatch",
            RF_STR_PF_ARG(intro)
        );
        return false;
    }

    if (got->type) {
        RFS_PUSH();
        ckr_compare_type(
            got->type,
            expect->type,
            file,
            line,
            RFS(RF_STR_PF_FMT". Value \""RF_STR_PF_FMT"\" type comparison failed.",
                RF_STR_PF_ARG(intro),
                RF_STR_PF_ARG(rir_value_string(got)))
        );
        RFS_POP();
    }

    switch(got->category) {
    case RIR_VALUE_CONSTANT:
        ckr_compare_constant(&got->constant, &expect->constant, file, line);
        break;
    case RIR_VALUE_LABEL:
        // can't compare dst_block since pointers will be different
        break;
    case RIR_VALUE_LITERAL:
        if (!rf_string_equal(&got->literal, &expect->literal)) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR value comparison",
                RF_STR_PF_FMT". Expected 'label_dst' to be %p but it is %p.",
                RF_STR_PF_ARG(intro),
                got->label_dst,
                expect->label_dst
            );
            return false;
        }
        break;
    default:
        // nothing to compare in other cases
        break;
    }

    return true;
}

static bool ckr_compare_valarr(
    struct value_arr *got,
    struct value_arr *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
)
{
    struct rir_value **gval;
    unsigned int i = 0;
    darray_foreach(gval, *got) {
        struct rir_value *eval = darray_item(*expect, i);
        if (!eval) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR value array comparison",
                RF_STR_PF_FMT". For the "RF_STR_PF_FMT" value in the array "
                "got a value but expected none",
                RF_STR_PF_ARG(intro),
                RF_STR_PF_ARG(rf_string_ordinal(i + 1))
            );
            return false;
        }
        ckr_compare_value(
            *gval,
            eval,
            file,
            line,
            RFS(RF_STR_PF_FMT". At the "RF_STR_PF_FMT "value in the array",
                RF_STR_PF_ARG(rf_string_ordinal(i + 1)))
        );
        i++;
    }
    return true;
}

static bool ckr_compare_object(
    struct rir_object *got,
    struct rir_object *expect,
    const char *file,
    unsigned int line
)
{
    if (got->category != expect->category) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR object comparison",
            "Expected 'category' to be \""RF_STR_PF_FMT"\" but "
            "it is \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rir_object_category_str(expect)),
            RF_STR_PF_ARG(rir_object_category_str(got))
        );
        return false;
    }
    switch(got->category) {
    case RIR_OBJ_EXPRESSION:
    case RIR_OBJ_BLOCK:
    case RIR_OBJ_TYPEDEF:
    case RIR_OBJ_GLOBAL:
        ck_abort_msg("ERROR: Not yet implemented check at 'ckr_compare_object'");
        break;
    case RIR_OBJ_VARIABLE:
        RFS_PUSH();
        ckr_compare_value(
            &got->variable.val,
            &expect->variable.val,
            file,
            line,
            RFS("At object comparison")
        );
        RFS_POP();
        break;

    }
    return true;
}

static bool ckr_compare_type(
    const struct rir_type *got,
    const struct rir_type *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
)
{
    if (expect->category != got->category) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR type comparison",
            RF_STR_PF_FMT"Expected 'category' to be \""RF_STR_PF_FMT"\" but "
            "it is \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(intro),
            RF_STR_PF_ARG(rir_type_category_str(expect)),
            RF_STR_PF_ARG(rir_type_category_str(got))
        );
        return false;
    }

    if (expect->is_pointer != got->is_pointer) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR type comparison",
            RF_STR_PF_FMT" Pointer mismatch.",
            RF_STR_PF_ARG(intro)
        );
        return false;
    }

    if (got->category == RIR_TYPE_ELEMENTARY) {
        if (got->etype != expect->etype) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR type comparison",
                RF_STR_PF_FMT" Elementary type mismatch. Expected \""
                RF_STR_PF_FMT"\" but got \""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(intro),
                RF_STR_PF_ARG(type_elementary_get_str(expect->etype)),
                RF_STR_PF_ARG(type_elementary_get_str(got->etype))
            );
            return false;
        }
    } else { // composite type
        if (!rir_typedef_equal(got->tdef, expect->tdef)) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR type comparison",
                RF_STR_PF_FMT" Composite type mismatch. Expected \""
                RF_STR_PF_FMT"\" but got \""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(intro),
                RF_STR_PF_ARG(&expect->tdef->name),
                RF_STR_PF_ARG(&got->tdef->name)
            );
            return false;
        }
    }
    return true;
}

static bool ckr_compare_arglist(
    struct rir_type_arr *got_arr,
    struct rir_type_arr *expect_arr,
    const struct RFstring *location_desc,
    const char *file,
    unsigned int line,
    unsigned int idx
)
{
    struct rir_type **got_t;
    unsigned int i = 0;
    darray_foreach(got_t, *got_arr) {
        struct rir_type *expect_t = darray_item(*expect_arr, i);
        if (!expect_t) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR argument array comparison",
                "For the "RF_STR_PF_FMT" got a "RF_STR_PF_FMT
                " argument but could not find such an argument in the expected"
                " results",
                RF_STR_PF_ARG(location_desc),
                RF_STR_PF_ARG(rf_string_ordinal(i + 1))
            );
            return false;
        }
        RFS_PUSH();
        ckr_compare_type(
            *got_t,
            expect_t,
            file,
            line,
            RFS("Failed to match the " RF_STR_PF_FMT " argument of "
                "\""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(rf_string_ordinal(i + 1)),
                RF_STR_PF_ARG(location_desc))
        );
        RFS_POP();
        i++;
    }
    return true;
}

static bool ckr_compare_typedef(
    struct rir_typedef *got,
    struct rir_typedef *expect,
    const char *file,
    unsigned int line,
    unsigned int idx
)
{
    if (!rf_string_equal(&got->name, &expect->name)) {
        ck_abort_at(
            file,
            line,
            "Failure at the RIR typedef comparison",
            "For the "RF_STR_PF_FMT" typedef expected 'name' to be "
            "\""RF_STR_PF_FMT"\" but got \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rf_string_ordinal(idx + 1)),
            RF_STR_PF_ARG(&expect->name),
            RF_STR_PF_ARG(&got->name)
        );
        return false;
    }

    if (got->is_union != expect->is_union) {
        ck_abort_at(
            file,
            line,
            "Failure at the RIR typedef comparison",
            "For the "RF_STR_PF_FMT" typedef expected 'is_union' to be "
            "\"%s\" but got \"%s\".",
            RF_STR_PF_ARG(rf_string_ordinal(idx + 1)),
            FMT_BOOL(got->is_union),
            FMT_BOOL(expect->is_union)
        );
        return false;
    }

    RFS_PUSH();
    ckr_compare_arglist(
        &got->argument_types,
        &expect->argument_types,
        RFS("\""RF_STR_PF_FMT"\" typedef", RF_STR_PF_ARG(&got->name)),
        file,
        line,
        idx
    );
    RFS_POP();
    return true;
}

static bool ckr_compare_returnstmt(
    struct rir_return *got,
    struct rir_return *expect,
    const char* file,
    unsigned int line,
    const struct RFstring *intro
)
{
    if (!got->val ^ !expect->val) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR return statement comparison",
            RF_STR_PF_FMT". Return statement value existence mismatch.",
            RF_STR_PF_ARG(intro)
        );
        return false;
    }
    ckr_compare_value(
        got->val,
        expect->val,
        file,
        line,
        RFS(RF_STR_PF_FMT" At the value of a return statement. ",
            RF_STR_PF_ARG(intro))
    );
    return true;
}

static bool ckr_compare_block(
    struct rir_block *got,
    struct rir_block *expect,
    const char *file,
    unsigned int line,
    unsigned int bl_idx,
    const struct RFstring *fn_name
)
{
    ckr_compare_value(
        &got->label,
        &expect->label,
        file,
        line,
        RFS("At the "RF_STR_PF_FMT" block of function \""RF_STR_PF_FMT"\"",
            RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
            RF_STR_PF_ARG(fn_name))
    );

    if (got->exit.type != expect->exit.type) {
        ck_abort_at(
            file,
            line,
            "Failure at rir block comparison",
            "At the "RF_STR_PF_FMT" block of function \""RF_STR_PF_FMT"\" "
            "expected blockexit of type \""RF_STR_PF_FMT"\" but got \""
            RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
            RF_STR_PF_ARG(fn_name),
            RF_STR_PF_ARG(rir_block_exit_type_str(&expect->exit)),
            RF_STR_PF_ARG(rir_block_exit_type_str(&got->exit))
        );
        return false;
    }

    switch (got->exit.type) {
    case RIR_BLOCK_EXIT_INVALID:
        ck_abort_at(file, line, "Failure at block comparison", "Got invalid block exit");
        break;
    case RIR_BLOCK_EXIT_BRANCH:
        ckr_compare_value(
            got->exit.branch.dst,
            expect->exit.branch.dst,
            file,
            line,
            RFS("At the branch of the "RF_STR_PF_FMT" block of function "
                "\""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        ckr_compare_value(
            got->exit.condbranch.cond,
            expect->exit.condbranch.cond,
            file,
            line,
            RFS("At the condition of condbranch of the "RF_STR_PF_FMT" block "
                "of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        ckr_compare_value(
            got->exit.condbranch.taken,
            expect->exit.condbranch.taken,
            file,
            line,
            RFS("At the taken of condbranch of the "RF_STR_PF_FMT" block "
                "of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        ckr_compare_value(
            got->exit.condbranch.fallthrough,
            expect->exit.condbranch.fallthrough,
            file,
            line,
            RFS("At the fallthrough of condbranch of the "RF_STR_PF_FMT" block "
                "of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        break;
    case RIR_BLOCK_EXIT_RETURN:
        ckr_compare_returnstmt(
            &got->exit.retstmt,
            &expect->exit.retstmt,
            file,
            line,
            RFS("At the "RF_STR_PF_FMT" block of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        break;
    }

    // now compare all expressions of the block
    struct rir_expression *gexpr;
    struct rir_expression *eexpr;
    unsigned int expr_idx = 0;
    rf_ilist_for_each(&got->expressions, gexpr, ln) {
        eexpr = rf_ilist_at(&expect->expressions, struct rir_expression, ln, expr_idx);
        if (!eexpr) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR block comparison",
                "Failed to retrieve the "RF_STR_PF_FMT" expression of the "
                RF_STR_PF_FMT" block of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(expr_idx + 1)),
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name)
            );
        }
        ckr_compare_expression(
            gexpr,
            eexpr,
            file,
            line,
            RFS("At the "RF_STR_PF_FMT" expression of the "
                RF_STR_PF_FMT" block of function \""RF_STR_PF_FMT"\"",
                RF_STR_PF_ARG(rf_string_ordinal(expr_idx + 1)),
                RF_STR_PF_ARG(rf_string_ordinal(bl_idx)),
                RF_STR_PF_ARG(fn_name))
        );
        expr_idx++;
    }

    return true;
}

static bool ckr_compare_expression(
    struct rir_expression *got,
    struct rir_expression *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
)
{
    if (got->type != expect->type) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR expression comparison",
            RF_STR_PF_FMT". Expected 'expression_type' to be \""RF_STR_PF_FMT
            "\" but got \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(intro),
            RF_STR_PF_ARG(rir_expression_type_string(expect)),
            RF_STR_PF_ARG(rir_expression_type_string(got))
        );
        return false;
    }

    switch(got->type) {
    case RIR_EXPRESSION_CALL:
        if (!rf_string_equal(&got->call.name, &expect->call.name)) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RF_STR_PF_FMT". Expected 'call.name' to be \""RF_STR_PF_FMT
                "\" but got \""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(intro),
                RF_STR_PF_ARG(&expect->call.name),
                RF_STR_PF_ARG(&got->call.name)
            );
            return false;
        }
        if (got->call.foreign != expect->call.foreign) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RF_STR_PF_FMT". Expected 'call.foreign' to be %s but got %s.",
                RF_STR_PF_ARG(intro),
                FMT_BOOL(expect->call.foreign),
                FMT_BOOL(got->call.foreign)
            );
            return false;
        }
        ckr_compare_valarr(
            &got->call.args,
            &expect->call.args,
            file,
            line,
            RFS(RF_STR_PF_FMT". At arguments array of a call", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_ALLOCA:
        if (got->alloca.alloc_location != expect->alloca.alloc_location) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RF_STR_PF_FMT". 'alloca.alloc_location' mismatch",
                RF_STR_PF_ARG(intro)
            );
            return false;
        }
        ckr_compare_type(
            got->alloca.type,
            expect->alloca.type,
            file,
            line,
            RFS(RF_STR_PF_FMT". At the type of an alloca", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_CONVERT:
        ckr_compare_value(
            got->convert.val,
            expect->convert.val,
            file,
            line,
            RFS(RF_STR_PF_FMT". At a convert expression", RF_STR_PF_ARG(intro))
        );
        ckr_compare_type(
            got->convert.type,
            expect->convert.type,
            file,
            line,
            RFS(RF_STR_PF_FMT". At a convert expression", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_WRITE:
        ckr_compare_value(
            got->write.memory,
            expect->write.memory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At a write() memory value", RF_STR_PF_ARG(intro))
        );
        ckr_compare_value(
            got->write.writeval,
            expect->write.writeval,
            file,
            line,
            RFS(RF_STR_PF_FMT". At a write() towrite value", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_READ:
        ckr_compare_value(
            got->read.memory,
            expect->read.memory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At a read() memory value", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_OBJMEMBERAT:
        ckr_compare_value(
            got->objmemberat.objmemory,
            expect->objmemberat.objmemory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At an objmemberat() memory value", RF_STR_PF_ARG(intro))
        );
        if (got->objmemberat.idx != expect->objmemberat.idx) {
            ck_abort_at(
                file,
                line,
                "Failure at a RIR objmemberat() comparison",
                RF_STR_PF_FMT" expected objmemberat.id  to be %u but it is %u.",
                RF_STR_PF_ARG(intro),
                expect->objmemberat.idx,
                got->objmemberat.idx
            );
            return false;
        }
        break;

    case RIR_EXPRESSION_SETUNIONIDX:
        ckr_compare_value(
            got->setunionidx.unimemory,
            expect->setunionidx.unimemory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At an setunionidx() memory value", RF_STR_PF_ARG(intro))
        );
        ckr_compare_value(
            got->setunionidx.idx,
            expect->setunionidx.idx,
            file,
            line,
            RFS(RF_STR_PF_FMT". At an setunionidx() idx", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_GETUNIONIDX:
        ckr_compare_value(
            got->getunionidx.unimemory,
            expect->getunionidx.unimemory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At an getunionidx() memory", RF_STR_PF_ARG(intro))
        );
        break;

    case RIR_EXPRESSION_UNIONMEMBERAT:
        ckr_compare_value(
            got->unionmemberat.unimemory,
            expect->unionmemberat.unimemory,
            file,
            line,
            RFS(RF_STR_PF_FMT". At an unionmemberat() memory", RF_STR_PF_ARG(intro))
        );
        if (got->unionmemberat.idx != expect->unionmemberat.idx) {
            ck_abort_at(
                file,
                line,
                "Failure at a RIR unionmemberat() comparison",
                RF_STR_PF_FMT" expected unionmemberat.id  to be %u but it is %u.",
                RF_STR_PF_ARG(intro),
                expect->unionmemberat.idx,
                got->unionmemberat.idx
            );
            return false;
        }
        break;

        // should not get to such a comparison
    case RIR_EXPRESSION_RETURN:
    case RIR_EXPRESSION_CONSTANT:

        // TODO: what to check here (?)
    case RIR_EXPRESSION_ADD:
    case RIR_EXPRESSION_SUB:
    case RIR_EXPRESSION_MUL:
    case RIR_EXPRESSION_DIV:
    case RIR_EXPRESSION_CMP_EQ:
    case RIR_EXPRESSION_CMP_NE:
    case RIR_EXPRESSION_CMP_GE:
    case RIR_EXPRESSION_CMP_GT:
    case RIR_EXPRESSION_CMP_LE:
    case RIR_EXPRESSION_CMP_LT:

        // not implemented
    case RIR_EXPRESSION_LOGIC_AND:
    case RIR_EXPRESSION_LOGIC_OR:
    case RIR_EXPRESSION_PLACEHOLDER:
        ck_assert_msg("Should never get here");
        break;
    }
    return true;
}

static bool ckr_compare_function(
    struct rir_fndef *got,
    struct rir_fndef *expect,
    const char *file,
    unsigned int line,
    unsigned int fn_idx
)
{
    if (!rf_string_equal(&got->decl.name, &expect->decl.name)) {
        ck_abort_at(
            file,
            line,
            "Failure at the RIR function comparison",
            "For the "RF_STR_PF_FMT" function expected 'name' to be "
            "\""RF_STR_PF_FMT"\" but got \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(rf_string_ordinal(fn_idx + 1)),
            RF_STR_PF_ARG(&expect->decl.name),
            RF_STR_PF_ARG(&got->decl.name)
        );
        return false;
    }

    RFS_PUSH();
    ckr_compare_arglist(
        &got->decl.argument_types,
        &expect->decl.argument_types,
        RFS("\""RF_STR_PF_FMT"\" function", RF_STR_PF_ARG(&got->decl.name)),
        file,
        line,
        fn_idx
    );
    RFS_POP();

    RFS_PUSH();
    ckr_compare_type(
        got->decl.return_type,
        expect->decl.return_type,
        file,
        line,
        RFS("Failed to match the return type of function \""RF_STR_PF_FMT"\".",
            RF_STR_PF_ARG(&got->decl.name))
    );
    RFS_POP();

    struct rir_object **gvar;
    int idx = 0;
    darray_foreach(gvar, got->variables) {
        struct rir_object *evar = darray_item(expect->variables, idx);
        if (!evar) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR function comparison",
                "Could not find the "RF_STR_PF_FMT" variable at the expected "
                "map of function \""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(rf_string_ordinal(idx)),
                RF_STR_PF_ARG(&got->decl.name)
            );
        }
        ckr_compare_object(*gvar, evar, file, line);
        idx++;
    }


    if (!got->retslot_val ^ !expect->retslot_val) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR function comparison",
            "The "RF_STR_PF_FMT" has a 'retslot_val' mismatch with expected",
            RF_STR_PF_ARG(rf_string_ordinal(fn_idx))
        );
        return false;
    }

    if (got->retslot_val) {
        RFS_PUSH();
        ckr_compare_value(
            got->retslot_val,
            expect->retslot_val,
            file,
            line,
            RFS("At function \""RF_STR_PF_FMT"\" retslot_val",
                RF_STR_PF_ARG(&got->decl.name))
        );
        RFS_POP();
    }

    struct rir_block **gblock;
    idx = 0;
    darray_foreach(gblock, got->blocks) {
        struct rir_block *eblock = darray_item(expect->blocks, idx);
        if (!eblock) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR function comparison",
                "Could not find the "RF_STR_PF_FMT" block at the expected "
                "block array of function \""RF_STR_PF_FMT"\".",
                RF_STR_PF_ARG(rf_string_ordinal(idx)),
                RF_STR_PF_ARG(&got->decl.name)
            );
        }
        ckr_compare_block(*gblock, eblock, file, line, idx, &got->decl.name);
        idx++;
    }


    return true;
}

struct ckr_cmplit_ctx {
    const char *file;
    unsigned int line;
    struct rirobj_strmap *expect_map;
};
#define CKR_CMPLIT_CTX_INIT(file_, line_, map_) { .file = file_, .line = line_, .expect_map = map_ }

static bool ckr_compare_literals(
    const struct RFstring *got_str,
    struct rir_object *got_obj,
    struct ckr_cmplit_ctx *ctx
)
{
    struct rir_object *expect_obj =  strmap_get(ctx->expect_map, got_str);
    if (!expect_obj) {
        ck_abort_at(
            ctx->file,
            ctx->line,
            "Failure at RIR global literals comparison",
            "Could not find literal \""RF_STR_PF_FMT"\" at the expected map.",
            RF_STR_PF_ARG(got_str)
        );
    }
    return testrir_compare_global(&got_obj->global, &expect_obj->global, ctx->file, ctx->line);
}

bool ck_assert_parserir_impl(
    const char *file,
    unsigned int line,
    struct rir *got
)
{
    struct rir_testdriver *tdr = get_rir_testdriver();
    struct rir *expect = darray_item(tdr->target_rirs, 0);

    if (!got || !expect) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR module compare",
            "Could not get the modules for comparison"
        );
    }

    //compare global string literals
    struct ckr_cmplit_ctx cmplit_ctx = CKR_CMPLIT_CTX_INIT(file, line, &expect->global_literals);
    strmap_iterate(&got->global_literals, (strmap_it_cb)ckr_compare_literals, &cmplit_ctx);

    // compare type definitions
    struct rir_typedef *gdef;
    struct rir_typedef *edef;
    unsigned int idx = 0;
    rf_ilist_for_each(&got->typedefs, gdef, ln) {
        edef = rf_ilist_at(&expect->typedefs, struct rir_typedef, ln, idx);
        if (!edef) {
            ck_abort_at(
                file, line, "Failure at RIR module compare",
                "Failed to retrieve the "RF_STR_PF_FMT" expected typedef",
                RF_STR_PF_ARG(rf_string_ordinal(idx + 1))
            );
        }
        ckr_compare_typedef(gdef, edef, file, line, idx);
        idx++;
    }

    // compare functions
    struct rir_fndecl *gfn;
    struct rir_fndecl *efn;
    idx = 0;
    rf_ilist_for_each(&got->functions, gfn, ln) {
        efn = rf_ilist_at(&expect->functions, struct rir_fndecl, ln, idx);
        if (!edef) {
            ck_abort_at(
                file, line, "Failure at RIR module compare",
                "Failed to retrieve the "RF_STR_PF_FMT" expected function",
                RF_STR_PF_ARG(rf_string_ordinal(idx + 1))
            );
        }
        ckr_compare_function(
            rir_fndecl_to_fndef(gfn),
            rir_fndecl_to_fndef(efn),
            file,
            line,
            idx
        );
        idx++;
    }
    return true;
}
