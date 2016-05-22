#include <ir/parser/rirparser.h>

#include <rfbase/string/conversion.h>

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
        if (!(retv = rir_map_getobj_value(&p->ctx.common, id))) {
            rirparser_synerr(
                p,
                token_get_start(tok),
                NULL,
                "Previously Undeclared identifier \""RFS_PF"\" "RFS_PF".",
                RFS_PA(id),
                RFS_PA(msg)
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
            "Expected a rir value but got token \""RFS_PF"\" "RFS_PF".",
            RFS_PA(rir_tokentype_to_str(rir_toktype(tok))),
            RFS_PA(msg)
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
                "as "RFS_PF " argument at " RFS_PF,
                RFS_PA(sord),
                RFS_PA(msg)
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
                "Expected either ',' or ')' after "RFS_PF" argument of " RFS_PF,
                RFS_PA(sord),
                RFS_PA(msg)
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

