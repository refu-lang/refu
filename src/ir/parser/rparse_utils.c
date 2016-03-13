#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_value.h>

bool rir_parse_instr_start(struct rir_parser *p, const struct RFstring *msg)
{
    // consume instruction token
    lexer_curr_token_advance(parser_lexer(p));
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a '(' after "RFS_PF".", RFS_PA(msg)
        );
        return false;
    }
    return true;
}

struct rir_value *rir_parse_val_and_comma(struct rir_parser *p, const struct RFstring *msg)
{
    struct rir_value *val = NULL;
    RFS_PUSH();
    if (!(val = rir_parse_value(p, RFS("at "RFS_PF, RFS_PA(msg))))) {
        goto end;
    }
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ',' after "RFS_PF".", RFS_PA(msg)
        );
        rir_value_destroy(val, RIR_VALUE_PARSING);
        val = NULL;
        goto end;
    }
    // success
end:
    RFS_POP();
    return val;
}

struct rir_type *rir_parse_type_and_comma(struct rir_parser *p, const struct RFstring *msg)
{
    struct rir_type *type = NULL;
    RFS_PUSH();
    if (!(type = rir_parse_type(p, msg))) {
        goto end;
    }
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ',' after "RFS_PF".", RFS_PA(msg)
        );
        rir_type_destroy(type, rir_parser_rir(p));
        type = NULL;
        goto end;
    }
    // success
end:
    RFS_POP();
    return type;
}
