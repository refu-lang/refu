#include <ir/parser/rirparser.h>

#include <ir/rir_convert.h>
#include <ir/rir_call.h>
#include <ir/rir_value.h>
#include <ir/rir_type.h>
#include <ir/rir_expression.h>
#include <utils/common_strings.h>

struct rir_object *rir_parse_convert(struct rir_parser *p)
{
    // consume 'convert'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_CONVERT))) {
        return NULL;
    }
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("first argument of convert()");
    struct rir_value *val = rir_parse_val_and_comma(p, &lmsg);
    if (!val) {
        return NULL;
    }

    static const struct RFstring lmsg2 = RF_STRING_STATIC_INIT("second argument of convert()");
    struct rir_type *type = rir_parse_type(p, &lmsg2);
    if (!type) {
        goto fail_destroy_val;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' at the end of 'convert'.");
        goto fail_destroy_type;
    }

    struct rir_object *cnv = rir_convert_create_obj(val, type, RIRPOS_PARSE, &p->ctx);
    if (!cnv) {
        goto fail_destroy_type;
    }
    return cnv;

fail_destroy_type:
    rir_type_destroy(type, rir_parser_rir(p));
fail_destroy_val:
    rir_value_destroy(val, RIR_VALUE_PARSING);
    return NULL;
}

struct rir_object *rir_parse_write(struct rir_parser *p)
{
    struct token *start = lexer_curr_token(parser_lexer(p));
    // consume 'write'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_WRITE))) {
        return false;
    }
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("first argument of write()");
    struct rir_type *type = rir_parse_type(p, &lmsg);
    if (!type) {
        return NULL;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ',' after first argument of 'write'.");
        goto fail_destroy_type;
    }

    static const struct RFstring lmsg2 = RF_STRING_STATIC_INIT("second argument of write()");
    struct rir_value *dstval = rir_parse_val_and_comma(p, &lmsg2);
    if (!dstval) {
        goto fail_destroy_type;
    }

    static const struct RFstring lmsg3 = RF_STRING_STATIC_INIT("third argument of write()");
    struct rir_value *srcval = rir_parse_value(p, &lmsg3);
    if (!srcval) {
        goto fail_destroy_dst;
    }

    struct token *end;
    if (!(end = lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN))) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' at the end of 'write'.");
        goto fail_destroy_src;
    }

    if (!rir_type_identical(type, dstval->type)) {
        RFS_PUSH();
        rirparser_synerr(
            p,
            token_get_start(start),
            token_get_end(end),
            "Type mismatch at arguments of 'write'. First argument's type is \""
            RFS_PF "\" but expected \""RFS_PF"\".",
            RFS_PA(rir_type_string(dstval->type)),
            RFS_PA(rir_type_string(type))
        );
        RFS_POP();
        goto fail_destroy_src;
    }

    struct rir_type *valtype = rir_type_create_from_other(type, rir_parser_rir(p), false);
    if (!rir_type_identical(valtype, srcval->type)) {
        RFS_PUSH();
        rirparser_synerr(
            p,
            token_get_start(start),
            token_get_end(end),
            "Type mismatch at arguments of 'write'. Second argument's type is"
            " \""RFS_PF "\" but expected \""RFS_PF"\".",
            RFS_PA(rir_type_string(srcval->type)),
            RFS_PA(rir_type_string(valtype))
        );
        RFS_POP();
        goto fail_destroy_src;
    }

    struct rir_object *wrt;
    if (!(wrt = rir_write_create_obj(dstval, srcval, RIRPOS_PARSE, &p->ctx))) {
        goto fail_destroy_src;
    }
    return wrt;

fail_destroy_src:
    rir_value_destroy(srcval, RIR_VALUE_PARSING);
fail_destroy_dst:
    rir_value_destroy(dstval, RIR_VALUE_PARSING);
fail_destroy_type:
    rir_type_destroy(type, rir_parser_rir(p));
    return NULL;
}

struct rir_object *rir_parse_read(struct rir_parser *p)
{
    // consume 'read'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_READ))) {
        return NULL;
    }
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("argument of read()");
    struct rir_value *val = rir_parse_value(p, &lmsg);
    if (!val) {
        return NULL;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' at the end of 'convert'.");
        goto fail_destroy_val;
    }

    struct rir_object *rd = rir_read_create_obj(val, RIRPOS_PARSE, &p->ctx);
    if (!rd) {
        goto fail_destroy_val;
    }
    return rd;

fail_destroy_val:
    rir_value_destroy(val, RIR_VALUE_PARSING);
    return NULL;
}

struct rir_object *rir_parse_call(struct rir_parser *p)
{
    // consume 'call'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_CALL))) {
        return NULL;
    }

    struct token *tok;
    if (!(tok = lexer_expect_token(parser_lexer(p), RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier as first argument of call()"
        );
        return NULL;
    }
    const struct RFstring *fnname = ast_identifier_str(tok->value.value.ast);

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ',' after the function name in call()."
        );
        return NULL;
    }

    if (!(tok = lexer_expect_token(parser_lexer(p), RIR_TOK_IDENTIFIER))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected an identifier as second argument of call()"
        );
        return NULL;
    }
    const struct RFstring *second_str = ast_identifier_str(tok->value.value.ast);
    bool is_foreign;
    if (rf_string_equal(second_str, &g_str_defined)) {
        is_foreign = false;
    } else if (rf_string_equal(second_str, &g_str_foreign)) {
        is_foreign = true;
    } else {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Invalid identifier \""RFS_PF"\" as second argument of call()."
            " Expected either \"foreign\" or \"defined\".",
            RFS_PA(second_str)
        );
        return NULL;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ',' after the second argument of call()."
        );
        return NULL;
    }

    struct value_arr arr;
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("call()");
    if (!rir_parse_valuearr(p, &arr, &lmsg)) {
        return NULL;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' at the end of call().");
        goto fail_destroy_arr;
    }

    struct rir_object *call = rir_call_create_obj(fnname, &arr, is_foreign, RIRPOS_PARSE, &p->ctx);
    if (!call) {
        goto fail_destroy_arr;
    }

    // success
    return call;

fail_destroy_arr:
    rir_valuearr_deinit(&arr, RIR_VALUE_PARSING);
    return NULL;
}
