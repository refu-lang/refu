#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ast/constants.h>
#include <ir/parser/rirtoken.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

#define RIR_TOK_IS_VALUE(i_tok_)                            \
    rir_toktype(i_tok_) == RIR_TOK_CONTANT_INTEGER ||       \
        rir_toktype(i_tok_) == RIR_TOK_CONSTANT_FLOAT ||    \
        rir_toktype(i_tok_) == RIR_TOK_IDENTIFIER_VARIABLE

struct rir_value *rir_parse_value(struct rir_parser *p, const struct RFstring *msg)
{
    struct rir_value *retv;
    struct token *tok;
    if (!(tok = lexer_lookahead(parser_lexer(p), 1))) {
        return NULL;
    }

    switch(rir_toktype(tok)) {
    case RIR_TOK_CONTANT_INTEGER:
    {
        int64_t v;
        if (!ast_constant_get_integer(&tok->value.value.ast->constant, &v)) {
            RF_ERROR("Failed to convert ast constant to int");
            return NULL;
        }
        retv = rir_constantval_create_fromint64(v, rir_parser_rir(p));
        break;
    }
    case RIR_TOK_CONSTANT_FLOAT:
    {
        double v;
        if (!ast_constant_get_float(&tok->value.value.ast->constant, &v)) {
            RF_ERROR("Failed to convert ast constant to float");
            return NULL;
        }
        retv = rir_constantval_create_fromint64(v, rir_parser_rir(p));
        break;
    }
    case RIR_TOK_IDENTIFIER_VARIABLE:
    {
        const struct RFstring *id = ast_identifier_str(tok->value.value.ast);
        retv = rir_object_value(rir_map_getobj(&p->ctx.common, id));
        if (!retv) {
            rirparser_synerr(
                p,
                token_get_start(tok),
                NULL,
                "Previously Undeclared identifier \""RF_STR_PF_FMT"\" "RF_STR_PF_FMT".",
                RF_STR_PF_ARG(id),
                RF_STR_PF_ARG(msg)
        );
            return NULL;
        }
        break;
    }
    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Expected a rir value but got token \""RF_STR_PF_FMT"\" "RF_STR_PF_FMT".",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok))),
            RF_STR_PF_ARG(msg)
        );
        return NULL;
    }

    // consume value token
    lexer_curr_token_advance(parser_lexer(p));
    return retv;
}

bool rir_parse_valuearr(struct rir_parser *p, struct value_arr *arr, const struct RFstring *msg)
{
    struct token *tok;
    if (!(tok = lexer_lookahead(parser_lexer(p), 1))) {
        return false;
    }
    darray_init(*arr);

    unsigned idx = 1;
    RFS_PUSH();
    while (RIR_TOK_IS_VALUE(tok)) {
        const struct RFstring *sord = rf_string_ordinal(idx);
        struct rir_value *v = rir_parse_value(
            p,
            RFS(
                "as "RF_STR_PF_FMT " argument at " RF_STR_PF_FMT,
                RF_STR_PF_ARG(sord),
                RF_STR_PF_ARG(msg)
            )
        );
        if (!v) {
            goto fail_free_arr;
        }

        tok = lexer_lookahead(parser_lexer(p), 1);
        if (!tok || (rir_toktype(tok) != RIR_TOK_SM_CPAREN &&
                     rir_toktype(tok) != RIR_TOK_SM_COMMA)) {
            rirparser_synerr(
                p,
                lexer_last_token_start(parser_lexer(p)),
                NULL,
                "Expected either ',' or ')' after "RF_STR_PF_FMT" argument of " RF_STR_PF_FMT,
                RF_STR_PF_ARG(sord),
                RF_STR_PF_ARG(msg)
            );
            goto fail_free_arr;
        }
        // add it to the value array
        darray_append(*arr, v);
        // check if we reached end of array
        if (rir_toktype(tok) == RIR_TOK_SM_CPAREN) {
            // succesfully exit the loop
            break;
        }
        // else consume the token and go to the next one
        tok = lexer_next_token(parser_lexer(p));
        idx++;
    }
    return true;

fail_free_arr:
    darray_free(*arr);
    return false;
}

