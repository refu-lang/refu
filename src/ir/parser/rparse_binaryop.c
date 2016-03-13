#include <ir/parser/rirparser.h>

#include <ir/rir_binaryop.h>

// mapping from rir token type to rir expression type
enum rir_expression_type rir_tok_to_boptype_map[] = {
    [RIR_TOK_ADD] = RIR_EXPRESSION_ADD,
    [RIR_TOK_SUB] = RIR_EXPRESSION_SUB,
    [RIR_TOK_MUL] = RIR_EXPRESSION_MUL,
    [RIR_TOK_DIV] = RIR_EXPRESSION_DIV,
    [RIR_TOK_CMPEQ] = RIR_EXPRESSION_CMP_EQ,
    [RIR_TOK_CMPNE] = RIR_EXPRESSION_CMP_NE,
    [RIR_TOK_CMPGT] = RIR_EXPRESSION_CMP_GT,
    [RIR_TOK_CMPGE] = RIR_EXPRESSION_CMP_GE,
    [RIR_TOK_CMPLT] = RIR_EXPRESSION_CMP_LT,
    [RIR_TOK_CMPLE] = RIR_EXPRESSION_CMP_LE,
};

static inline enum rir_expression_type rir_toktype_to_boptype(enum rir_token_type tok_type)
{
    return rir_tok_to_boptype_map[tok_type];
}

struct rir_object *rir_parse_binaryop(struct rir_parser *p, enum rir_token_type tok_type)
{
    enum rir_expression_type bop_type = rir_toktype_to_boptype(tok_type);
    // consume binary op instruction
    if (!rir_parse_instr_start(p, rir_binaryoptype_string(bop_type))) {
        return false;
    }

    RFS_PUSH();
    struct token *tok = lexer_curr_token(parser_lexer(p));
    const struct inplocation_mark *t_start = token_get_start(tok);
    const struct inplocation_mark *t_end = token_get_end(tok);
    struct rir_type *parsed_type = rir_parse_type_and_comma(
        p,
        RFS("type of \""RFS_PF"\"", RFS_PA(rir_binaryoptype_string(bop_type)))
    );
    if (!parsed_type) {
        goto fail;
    }

    struct rir_value *vala = rir_parse_val_and_comma(
        p,
        RFS("first argument of "RFS_PF"()", RFS_PA(rir_binaryoptype_string(bop_type)))
    );
    if (!vala) {
        goto fail_free_type;
    }

    struct rir_value *valb = rir_parse_value(
        p,
        RFS("first argument of "RFS_PF"()", RFS_PA(rir_binaryoptype_string(bop_type)))
    );
    if (!valb) {
        goto fail_free_vala;
    }

    // check that the parsed binary op type is correct
    if (!rir_type_identical(parsed_type, vala->type)) {
        RFS_PUSH();
        rirparser_synerr(
            p,
            t_start,
            t_end,
            "Type mismatch at arguments of \""RFS_PF"\". First argument's type"
            " is \""RFS_PF "\" but expected \""RFS_PF"\".",
            RFS_PA(rir_binaryoptype_string(bop_type)),
            RFS_PA(rir_type_string(vala->type)),
            RFS_PA(rir_type_string(parsed_type))
        );
        RFS_POP();
        goto fail_free_valb;
    }
    if (!rir_type_identical(parsed_type, valb->type)) {
        RFS_PUSH();
        rirparser_synerr(
            p,
            t_start,
            t_end,
            "Type mismatch at arguments of \""RFS_PF"\". Second argument's type"
            " is \""RFS_PF "\" but expected \""RFS_PF"\".",
            RFS_PA(rir_binaryoptype_string(bop_type)),
            RFS_PA(rir_type_string(valb->type)),
            RFS_PA(rir_type_string(parsed_type))
        );
        RFS_POP();
        goto fail_free_valb;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
            NULL,
            "Expected a ')' at the end of \""RFS_PF"\"'.",
            RFS_PA(rir_binaryoptype_string(bop_type))
        );
        goto fail_free_valb;
    }

    struct rir_object *bop = rir_binaryop_create_nonast_obj(
        bop_type,
        vala,
        valb,
        RIRPOS_PARSE,
        &p->ctx
    );
    if (!bop) {
        goto fail_free_valb;
    }

    RFS_POP();
    return bop;

fail_free_valb:
    rir_value_destroy(valb, RIR_VALUE_PARSING);
fail_free_vala:
    rir_value_destroy(vala, RIR_VALUE_PARSING);
fail_free_type:
    rir_type_destroy(parsed_type, rir_parser_rir(p));
fail:
    RFS_POP();
    return NULL;
}
