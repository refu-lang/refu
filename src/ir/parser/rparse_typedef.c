#include "rparse_functions.h"

#include <ir/parser/rirparser.h>
#include <lexer/lexer.h>
#include <ir/rir_global.h>
#include <ir/rir_object.h>
#include <ir/rir.h>

bool rir_parse_typearr(struct rir_parser *p, struct rir_type_arr *arr, struct rir *r)
{
    struct token *tok = lexer_lookahead(&p->lexer, 1);
    bool end_reached = false;
    if (!tok) {
        return false;
    }
    darray_init(*arr);

    struct rir_type *t;
    while (rir_toktype(tok) == RIR_TOK_IDENTIFIER && !end_reached) {        
        if (!(t = rir_type_byname(r, ast_identifier_str(tok->value.value.ast)))) {
            rirparser_synerr(p, token_get_start(tok), NULL,
                             "Provided string is not a recognized type");
            goto fail_free_arr;
        }

        // consume type identifier
        tok = lexer_gnext_token(&p->lexer);
        if (!tok || (rir_toktype(tok) != RIR_TOK_OP_MULTI &&
                     rir_toktype(tok) != RIR_TOK_SM_COMMA &&
                     rir_toktype(tok) != RIR_TOK_SM_CPAREN)) {
            rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                             "Expected either '*', ',' or ')' in the type array");
            goto fail_free_arr;
        }

        if (rir_toktype(tok) == RIR_TOK_OP_MULTI) {
            t->is_pointer = true;
            // consume '*'
            tok = lexer_gnext_token(&p->lexer);
        }

        if (!tok || (rir_toktype(tok) != RIR_TOK_SM_CPAREN &&
                     rir_toktype(tok) != RIR_TOK_SM_COMMA)) {
            rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                             "Expected either ',' or ')' after type identifier");
            goto fail_free_arr;
        }
        // add it to the type array
        darray_append(*arr, t);

        // check if we reached end of array
        end_reached = rir_toktype(tok) == RIR_TOK_SM_CPAREN;
        // consume the token and go to the next one
        tok = lexer_gnext_token(&p->lexer);
    }
    return true;

fail_free_arr:
    darray_free(*arr);
    return false;
}

bool rir_parse_typedef(struct rir_parser *p, struct token *id, bool uniondef, struct rir *r)
{
#define i_DEFSTR "'%s'.", uniondef ? "uniondef" : "typedef"

    // consume identifier_variable, =, uniondef
    lexer_next_token(&p->lexer);
    lexer_next_token(&p->lexer);
    lexer_next_token(&p->lexer);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                      "Expected '(' after "i_DEFSTR);
        return false;
    }

    // initialize a typedef here
    struct rir_typedef *def;
    RF_MALLOC(def, sizeof(*def), return false);
    def->is_union = uniondef;
    if (!(def->name = rf_string_copy_out(ast_identifier_str(id->value.value.ast)))) {
        goto fail_free_def;
    }

    // parse the type array into the typedef
    if (!rir_parse_typearr(p, &def->argument_types, r)) {
        goto fail_free_name;
    }

    // by now typedef is ready so add it to the rir typedefs list
    rf_ilist_add_tail(&r->typedefs, &def->ln);

#undef i_typestr
    return true;

fail_free_name:
    rf_string_destroy(def->name);
fail_free_def:
    free(def);
    return false;
}
