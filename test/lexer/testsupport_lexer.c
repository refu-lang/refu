#include "testsupport_lexer.h"

static bool tokens_cmp(struct token *t1, struct token *t2, unsigned int index,
                       const char *filename, unsigned int line)
{

    if (t1->type != t2->type) {
        ck_lexer_abort(filename, line, "Expected the %d token to be of type "
                       "\""RF_STR_PF_FMT"\" but it was \""RF_STR_PF_FMT"\"",
                       index, RF_STR_PF_ARG(tokentype_to_str(t1->type)),
                       RF_STR_PF_ARG(tokentype_to_str(t2->type)));
        return false;
    }

    if (!ast_location_equal(&t1->loc, &t2->loc)) {
        ck_lexer_abort(filename, line,
                       "Expected token %d to have location:\n"
                       AST_LOCATION_FMT2"\nbut it has location:\n"
                       AST_LOCATION_FMT2, index, AST_LOCATION_ARG2(&t1->loc),
                       AST_LOCATION_ARG2(&t2->loc));
        return false;
    }


    if (t1->type == TOKEN_IDENTIFIER &&
        !rf_string_equal(&t1->value.string, &t2->value.string)) {
        ck_lexer_abort(filename, line,
                       "Expected the %d token to have value:\n"
                       RF_STR_PF_FMT"\nbut it has value:\n"
                       RF_STR_PF_FMT, index, RF_STR_PF_ARG(&t1->value.string),
                       RF_STR_PF_ARG(&t2->value.string));
        return false;
    }

    return true;
}


void check_lexer_tokens_impl(struct lexer *l,
                             struct token *tokens,
                             unsigned num,
                             const char *filename,
                             unsigned int line)
{
    struct token *t;
    unsigned int i = 0;
    if (darray_size(l->tokens) != num) {
        ck_lexer_abort(filename, line, "Expected %d tokens but got %d",
                       num, darray_size(l->tokens));
    }


    darray_foreach(t, l->tokens) {
        tokens_cmp(&tokens[i], t, i, filename, line);
        i ++;
    }
}
