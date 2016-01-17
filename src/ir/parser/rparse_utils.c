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
            "Expected a '(' after "RF_STR_PF_FMT".", RF_STR_PF_ARG(msg)
        );
        return false;
    }
    return true;
}

struct rir_value *rir_parse_val_and_comma(struct rir_parser *p, const struct RFstring *msg)
{
    struct rir_value *val = NULL;
    RFS_PUSH();
    if (!(val = rir_parse_value(p, RFS("at "RF_STR_PF_FMT, RF_STR_PF_ARG(msg))))) {
        goto end;
    }
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ',' after "RF_STR_PF_FMT".", RF_STR_PF_ARG(msg)
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
