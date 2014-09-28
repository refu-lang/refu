#include "testsupport_lexer.h"

#include <Utils/constcmp.h>

static bool tokens_cmp(struct token *expected,
                       struct token *got,
                       unsigned int index,
                       struct inpfile *f,
                       const char *filename,
                       unsigned int line)
{

    if (expected->type != got->type) {
        ck_lexer_abort(filename, line, "Expected the %d token to be of type "
                       "\""RF_STR_PF_FMT"\" but it was \""RF_STR_PF_FMT"\"",
                       index, RF_STR_PF_ARG(tokentype_to_str(expected->type)),
                       RF_STR_PF_ARG(tokentype_to_str(got->type)));
        return false;
    }

    if (!inplocation_equal(token_get_loc(expected), token_get_loc(got))) {
        ck_lexer_abort(filename, line,
                       "Expected token %d to have location:\n"
                       INPLOCATION_FMT2"\nbut it has location:\n"
                       INPLOCATION_FMT2, index,
                       INPLOCATION_ARG2(f, token_get_loc(expected)),
                       INPLOCATION_ARG2(f, token_get_loc(got)));
        return false;
    }


    if (expected->type == TOKEN_IDENTIFIER &&
        !rf_string_equal(
            ast_identifier_str(expected->value.identifier.id),
            ast_identifier_str(got->value.identifier.id))) {
        ck_lexer_abort(
            filename, line,
            "Expected the %d token to have value:\n"
            RF_STR_PF_FMT"\nbut it has value:\n"
            RF_STR_PF_FMT, index,
            RF_STR_PF_ARG(ast_identifier_str(expected->value.identifier.id)),
            RF_STR_PF_ARG(ast_identifier_str(got->value.identifier.id)));
        return false;
    } else if (expected->type == TOKEN_CONSTANT_INTEGER &&
               expected->value.int_constant != got->value.int_constant) {

                ck_lexer_abort(
                    filename, line,
                    "Expected the %d token to have value:\n"
                    PRIu64"\nbut it has value:\n"
                    PRIu64, index,
                    expected->value.int_constant, got->value.int_constant);
    } else if (
        expected->type == TOKEN_CONSTANT_FLOAT &&
        !DBLCMP_EQ(expected->value.float_constant, got->value.float_constant)) {

                ck_lexer_abort(
                    filename, line,
                    "Expected the %d token to have value:\n"
                    "%f\nbut it has value:\n%f",
                    index,
                    expected->value.float_constant, got->value.float_constant);
    } else if (expected->type == TOKEN_STRING_LITERAL &&
               !rf_string_equal(&expected->value.literal, &got->value.literal)) {

        ck_lexer_abort(
            filename, line,
            "Expected the %d string literal token to have value:\n"
            "\""RF_STR_PF_FMT"\"\nbut it has value:\n"
            "\""RF_STR_PF_FMT"\"", index,
            RF_STR_PF_ARG(&expected->value.literal),
            RF_STR_PF_ARG(&got->value.literal));
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
        tokens_cmp(&tokens[i], t, i, l->file, filename, line);
        i ++;
    }
}
