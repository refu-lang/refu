#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ast/constants.h>
#include <ir/parser/rirtoken.h>
#include <ir/rir_value.h>
#include <ir/rir_constant.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

struct rir_value *rir_parse_value(struct rir_parser *p, const struct RFstring *msg)
{
    struct rir_value *retv;
    struct token *tok;
    if (!(tok = lexer_lookahead(&p->lexer, 1))) {
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
    lexer_curr_token_advance(&p->lexer);
    return retv;
}
