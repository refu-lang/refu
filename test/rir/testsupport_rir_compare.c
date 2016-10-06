#include "testsupport_rir_compare.h"

#include "../testsupport.h"
#include "testsupport_rir.h"
#include <rfbase/string/core.h>
#include <ast/constants.h>
#include <ir/rir_expression.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir_function.h>

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
            "\""RFS_PF"\" but got one named \""RFS_PF"\".",
            RFS_PA(expect_name),
            RFS_PA(got_name)
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
            "\""RFS_PF"\" but got one with type \""RFS_PF"\".",
            RFS_PA(rir_type_string(expect_type)),
            RFS_PA(rir_type_string(got_type))
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
            "Expected 'type' to be \""RFS_PF"\" but "
            "it is \""RFS_PF"\".",
            RFS_PA(ast_constant_type_string(expect)),
            RFS_PA(ast_constant_type_string(got))
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
    if (got == NULL && expect == NULL) { // both values non-existing
        return true;
    }

    if (got->category != expect->category) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RFS_PF". Expected 'category' to be \""RFS_PF"\" but "
            "it is \""RFS_PF"\".",
            RFS_PA(intro),
            RFS_PA(rir_value_type_string(expect)),
            RFS_PA(rir_value_type_string(got))
        );
        return false;
    }

    if (!rf_string_equal(&got->id, &expect->id)) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RFS_PF". Expected 'id' to be \""RFS_PF"\" but "
            "it is \""RFS_PF"\".",
            RFS_PA(intro),
            RFS_PA(rir_value_string(expect)),
            RFS_PA(rir_value_string(got))
        );
        return false;
    }

    if (!rir_type_identical(got->type, expect->type)) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR value comparison",
            RFS_PF". Value type existence mismatch",
            RFS_PA(intro)
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
            RFS(RFS_PF". Value \""RFS_PF"\" type comparison failed.",
                RFS_PA(intro),
                RFS_PA(rir_value_string(got)))
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
                RFS_PF". Expected 'label_dst' to be %p but it is %p.",
                RFS_PA(intro),
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
                RFS_PF". For the "RFS_PF" value in the array "
                "got a value but expected none",
                RFS_PA(intro),
                RFS_PA(rf_string_ordinal(i + 1))
            );
            return false;
        }
        RFS_PUSH();
        ckr_compare_value(
            *gval,
            eval,
            file,
            line,
            RFS(RFS_PF". At the "RFS_PF "value in the array",
                RFS_PA(intro),
                RFS_PA(rf_string_ordinal(i + 1)))
        );
        RFS_POP();
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
            "Expected 'category' to be \""RFS_PF"\" but "
            "it is \""RFS_PF"\".",
            RFS_PA(rir_object_category_str(expect)),
            RFS_PA(rir_object_category_str(got))
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

bool ckr_compare_type(
    const struct rir_type *got,
    const struct rir_type *expect,
    const char *file,
    unsigned int line,
    const struct RFstring *intro
)
{
    if (!got ^ !expect) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR type comparison",
            RFS_PF". Type existence mismatch.",
            RFS_PA(intro)
        );
        return false;
    }
    if (!got) { // comparing 2 null types
        return true;
    }

    if (expect->category != got->category) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR type comparison",
            RFS_PF"Expected 'category' to be \""RFS_PF"\" but "
            "it is \""RFS_PF"\".",
            RFS_PA(intro),
            RFS_PA(rir_type_category_str(expect)),
            RFS_PA(rir_type_category_str(got))
        );
        return false;
    }

    if (expect->is_pointer != got->is_pointer) {
        ck_abort_at(
            file,
            line,
            "Failure at RIR type comparison",
            RFS_PF" Pointer mismatch.",
            RFS_PA(intro)
        );
        return false;
    }

    if (got->category == RIR_TYPE_ELEMENTARY) {
        if (got->etype != expect->etype) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR type comparison",
                RFS_PF" Elementary type mismatch. Expected \""
                RFS_PF"\" but got \""RFS_PF"\".",
                RFS_PA(intro),
                RFS_PA(type_elementary_get_str(expect->etype)),
                RFS_PA(type_elementary_get_str(got->etype))
            );
            return false;
        }
    } else { // composite type
        if (!rir_typedef_equal(got->tdef, expect->tdef)) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR type comparison",
                RFS_PF" Composite type mismatch. Expected \""
                RFS_PF"\" but got \""RFS_PF"\".",
                RFS_PA(intro),
                RFS_PA(&expect->tdef->name),
                RFS_PA(&got->tdef->name)
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
                "For the "RFS_PF" got a "RFS_PF
                " argument but could not find such an argument in the expected"
                " results",
                RFS_PA(location_desc),
                RFS_PA(rf_string_ordinal(i + 1))
            );
            return false;
        }
        RFS_PUSH();
        ckr_compare_type(
            *got_t,
            expect_t,
            file,
            line,
            RFS("Failed to match the " RFS_PF " argument of "
                "\""RFS_PF"\".",
                RFS_PA(rf_string_ordinal(i + 1)),
                RFS_PA(location_desc))
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
            "For the "RFS_PF" typedef expected 'name' to be "
            "\""RFS_PF"\" but got \""RFS_PF"\".",
            RFS_PA(rf_string_ordinal(idx + 1)),
            RFS_PA(&expect->name),
            RFS_PA(&got->name)
        );
        return false;
    }

    if (got->is_union != expect->is_union) {
        ck_abort_at(
            file,
            line,
            "Failure at the RIR typedef comparison",
            "For the "RFS_PF" typedef expected 'is_union' to be "
            "\"%s\" but got \"%s\".",
            RFS_PA(rf_string_ordinal(idx + 1)),
            FMT_BOOL(got->is_union),
            FMT_BOOL(expect->is_union)
        );
        return false;
    }

    RFS_PUSH();
    ckr_compare_arglist(
        &got->argument_types,
        &expect->argument_types,
        RFS("\""RFS_PF"\" typedef", RFS_PA(&got->name)),
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
            RFS_PF". Return statement value existence mismatch.",
            RFS_PA(intro)
        );
        return false;
    }
    ckr_compare_value(
        got->val,
        expect->val,
        file,
        line,
        RFS(RFS_PF" At the value of a return statement. ",
            RFS_PA(intro))
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
    RFS_PUSH();
    ckr_compare_value(
        &got->label,
        &expect->label,
        file,
        line,
        RFS("At the "RFS_PF" block of function \""RFS_PF"\"",
            RFS_PA(rf_string_ordinal(bl_idx)),
            RFS_PA(fn_name))
    );
    RFS_POP();

    if (got->exit.type != expect->exit.type) {
        ck_abort_at(
            file,
            line,
            "Failure at rir block comparison",
            "At the "RFS_PF" block of function \""RFS_PF"\" "
            "expected blockexit of type \""RFS_PF"\" but got \""
            RFS_PF"\".",
            RFS_PA(rf_string_ordinal(bl_idx)),
            RFS_PA(fn_name),
            RFS_PA(rir_block_exit_type_str(&expect->exit)),
            RFS_PA(rir_block_exit_type_str(&got->exit))
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
            RFS("At the branch of the "RFS_PF" block of function "
                "\""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
        );
        break;
    case RIR_BLOCK_EXIT_CONDBRANCH:
        ckr_compare_value(
            got->exit.condbranch.cond,
            expect->exit.condbranch.cond,
            file,
            line,
            RFS("At the condition of condbranch of the "RFS_PF" block "
                "of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
        );
        ckr_compare_value(
            got->exit.condbranch.taken,
            expect->exit.condbranch.taken,
            file,
            line,
            RFS("At the taken of condbranch of the "RFS_PF" block "
                "of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
        );
        ckr_compare_value(
            got->exit.condbranch.fallthrough,
            expect->exit.condbranch.fallthrough,
            file,
            line,
            RFS("At the fallthrough of condbranch of the "RFS_PF" block "
                "of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
        );
        break;
    case RIR_BLOCK_EXIT_RETURN:
        ckr_compare_returnstmt(
            &got->exit.retstmt,
            &expect->exit.retstmt,
            file,
            line,
            RFS("At the "RFS_PF" block of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
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
                "Failed to retrieve the "RFS_PF" expression of the "
                RFS_PF" block of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(expr_idx + 1)),
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name)
            );
        }
        RFS_PUSH();
        ckr_compare_expression(
            gexpr,
            eexpr,
            file,
            line,
            RFS("At the "RFS_PF" expression of the "
                RFS_PF" block of function \""RFS_PF"\"",
                RFS_PA(rf_string_ordinal(expr_idx + 1)),
                RFS_PA(rf_string_ordinal(bl_idx)),
                RFS_PA(fn_name))
        );
        RFS_POP();
        expr_idx++;
    }

    return true;
}

bool ckr_compare_expression(
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
            RFS_PF". Expected 'expression_type' to be \""RFS_PF
            "\" but got \""RFS_PF"\".",
            RFS_PA(intro),
            RFS_PA(rir_expression_type_string(expect)),
            RFS_PA(rir_expression_type_string(got))
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
                RFS_PF". Expected 'call.name' to be \""RFS_PF
                "\" but got \""RFS_PF"\".",
                RFS_PA(intro),
                RFS_PA(&expect->call.name),
                RFS_PA(&got->call.name)
            );
            return false;
        }
        if (got->call.foreign != expect->call.foreign) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RFS_PF". Expected 'call.foreign' to be %s but got %s.",
                RFS_PA(intro),
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
            RFS(RFS_PF". At arguments array of a call", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_FIXEDARR:
        ckr_compare_type(
            got->fixedarr.member_type,
            expect->fixedarr.member_type,
            file,
            line,
            RFS(RFS_PF". At a fixedarr expression", RFS_PA(intro))
        );
        if (got->fixedarr.size != expect->fixedarr.size) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RFS_PF". Expected 'fixedarr.size' to be %"PRIu64" but got %"PRIu64".",
                RFS_PA(intro),
                FMT_BOOL(expect->fixedarr.size),
                FMT_BOOL(got->fixedarr.size)
            );
            return false;
        }
        ckr_compare_valarr(
            &got->fixedarr.members,
            &expect->fixedarr.members,
            file,
            line,
            RFS(RFS_PF". At members array of a fixedarr", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_ALLOCA:
        if (got->alloca.alloc_location != expect->alloca.alloc_location) {
            ck_abort_at(
                file,
                line,
                "Failure at RIR expression comparison",
                RFS_PF". 'alloca.alloc_location' mismatch",
                RFS_PA(intro)
            );
            return false;
        }
        ckr_compare_type(
            got->alloca.type,
            expect->alloca.type,
            file,
            line,
            RFS(RFS_PF". At the type of an alloca", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_CONVERT:
        ckr_compare_value(
            got->convert.val,
            expect->convert.val,
            file,
            line,
            RFS(RFS_PF". At a convert expression", RFS_PA(intro))
        );
        ckr_compare_type(
            got->convert.type,
            expect->convert.type,
            file,
            line,
            RFS(RFS_PF". At a convert expression", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_WRITE:
        ckr_compare_value(
            got->write.memory,
            expect->write.memory,
            file,
            line,
            RFS(RFS_PF". At a write() memory value", RFS_PA(intro))
        );
        ckr_compare_value(
            got->write.writeval,
            expect->write.writeval,
            file,
            line,
            RFS(RFS_PF". At a write() towrite value", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_READ:
        ckr_compare_value(
            got->read.memory,
            expect->read.memory,
            file,
            line,
            RFS(RFS_PF". At a read() memory value", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_OBJMEMBERAT:
        ckr_compare_value(
            got->objmemberat.objmemory,
            expect->objmemberat.objmemory,
            file,
            line,
            RFS(RFS_PF". At an objmemberat() memory value", RFS_PA(intro))
        );
        if (got->objmemberat.idx != expect->objmemberat.idx) {
            ck_abort_at(
                file,
                line,
                "Failure at a RIR objmemberat() comparison",
                RFS_PF" expected objmemberat.id  to be %u but it is %u.",
                RFS_PA(intro),
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
            RFS(RFS_PF". At an setunionidx() memory value", RFS_PA(intro))
        );
        ckr_compare_value(
            got->setunionidx.idx,
            expect->setunionidx.idx,
            file,
            line,
            RFS(RFS_PF". At an setunionidx() idx", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_GETUNIONIDX:
        ckr_compare_value(
            got->getunionidx.unimemory,
            expect->getunionidx.unimemory,
            file,
            line,
            RFS(RFS_PF". At an getunionidx() memory", RFS_PA(intro))
        );
        break;

    case RIR_EXPRESSION_UNIONMEMBERAT:
        ckr_compare_value(
            got->unionmemberat.unimemory,
            expect->unionmemberat.unimemory,
            file,
            line,
            RFS(RFS_PF". At an unionmemberat() memory", RFS_PA(intro))
        );
        if (got->unionmemberat.idx != expect->unionmemberat.idx) {
            ck_abort_at(
                file,
                line,
                "Failure at a RIR unionmemberat() comparison",
                RFS_PF" expected unionmemberat.id  to be %u but it is %u.",
                RFS_PA(intro),
                expect->unionmemberat.idx,
                got->unionmemberat.idx
            );
            return false;
        }
        break;

    case RIR_EXPRESSION_OBJIDX:
        ckr_compare_value(
            got->objidx.objmemory,
            expect->objidx.objmemory,
            file,
            line,
            RFS(RFS_PF". At an objidx() memory value", RFS_PA(intro))
        );
        ckr_compare_value(
            got->objidx.idx,
            expect->objidx.idx,
            file,
            line,
            RFS(RFS_PF". At an objidx() index value", RFS_PA(intro))
        );
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
            "For the "RFS_PF" function expected 'name' to be "
            "\""RFS_PF"\" but got \""RFS_PF"\".",
            RFS_PA(rf_string_ordinal(fn_idx + 1)),
            RFS_PA(&expect->decl.name),
            RFS_PA(&got->decl.name)
        );
        return false;
    }

    RFS_PUSH();
    ckr_compare_arglist(
        &got->decl.argument_types,
        &expect->decl.argument_types,
        RFS("\""RFS_PF"\" function", RFS_PA(&got->decl.name)),
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
        RFS("Failed to match the return type of function \""RFS_PF"\".",
            RFS_PA(&got->decl.name))
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
                "Could not find the "RFS_PF" variable at the expected "
                "map of function \""RFS_PF"\".",
                RFS_PA(rf_string_ordinal(idx)),
                RFS_PA(&got->decl.name)
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
            "The "RFS_PF" has a 'retslot_val' mismatch with expected",
            RFS_PA(rf_string_ordinal(fn_idx))
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
            RFS("At function \""RFS_PF"\" retslot_val",
                RFS_PA(&got->decl.name))
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
                "Could not find the "RFS_PF" block at the expected "
                "block array of function \""RFS_PF"\".",
                RFS_PA(rf_string_ordinal(idx)),
                RFS_PA(&got->decl.name)
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
            "Could not find literal \""RFS_PF"\" at the expected map.",
            RFS_PA(got_str)
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
                "Failed to retrieve the "RFS_PF" expected typedef",
                RFS_PA(rf_string_ordinal(idx + 1))
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
        if (!efn) {
            ck_abort_at(
                file, line, "Failure at RIR module compare",
                "Failed to retrieve the "RFS_PF" expected function",
                RFS_PA(rf_string_ordinal(idx + 1))
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
