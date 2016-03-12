#include "testsupport_lexer.h"

#include <Utils/constcmp.h>

#include <ast/constants.h>
#include <ast/string_literal.h>

bool test_tokens_cmp(struct token *expected,
                     struct token *got,
                     unsigned int index,
                     struct inpfile *f,
                     const char *filename,
                     unsigned int line)
{

    if (expected->type != got->type) {
        ck_lexer_abort(
            filename, line,
            "Expected the %d token to be of type "
            "\""RFS_PF"\" but it was \""RFS_PF"\"",
            index, RFS_PA(tokentype_to_str(expected->type)),
            RFS_PA(tokentype_to_str(got->type))
        );
        return false;
    }

    if (!inplocation_equal(token_get_loc(expected), token_get_loc(got))) {
        ck_lexer_abort(
            filename, line,
            "Expected token %d to have location:\n"
            INPLOCATION_FMT2"\nbut it has location:\n"
            INPLOCATION_FMT2, index,
            INPLOCATION_ARG2(f, token_get_loc(expected)),
            INPLOCATION_ARG2(f, token_get_loc(got))
        );
        return false;
    }


    if (expected->type == TOKEN_IDENTIFIER &&
        !rf_string_equal(
            ast_identifier_str(expected->value.value.ast),
            ast_identifier_str(got->value.value.ast))) {
        ck_lexer_abort(
            filename, line,
            "Expected the %d token to have value:\n"
            RFS_PF"\nbut it has value:\n"
            RFS_PF, index,
            RFS_PA(ast_identifier_str(expected->value.value.ast)),
            RFS_PA(ast_identifier_str(got->value.value.ast)));
        return false;
    } else if (expected->type == TOKEN_CONSTANT_INTEGER) {
        int64_t expect_v;
        int64_t got_v;
        ck_assert(ast_constant_get_integer(&expected->value.value.ast->constant, &expect_v));
        ck_assert(ast_constant_get_integer(&got->value.value.ast->constant, &got_v));
        if (expect_v != got_v) {
                ck_lexer_abort(
                    filename, line,
                    "Expected the %d token to have value:\n"
                    "%"PRIu64"\nbut it has value:\n"
                    "%"PRIu64, index,
                    expect_v, got_v);
        }
    } else if (expected->type == TOKEN_CONSTANT_FLOAT) {
        double expect_v;
        double got_v;
        ck_assert(ast_constant_get_float(&expected->value.value.ast->constant, &expect_v));
        ck_assert(ast_constant_get_float(&got->value.value.ast->constant, &got_v));
        if (!DBLCMP_EQ(expect_v, got_v)) {
                ck_lexer_abort(
                    filename, line,
                    "Expected the %d token to have value:\n"
                    "%f\nbut it has value:\n%f",
                    index,
                    expect_v, got_v);
        }
    } else if (expected->type == TOKEN_STRING_LITERAL &&
               !rf_string_equal(
                   ast_string_literal_get_str(expected->value.value.ast),
                   ast_string_literal_get_str(got->value.value.ast))) {

        ck_lexer_abort(
            filename, line,
            "Expected the %d string literal token to have value:\n"
            "\""RFS_PF"\"\nbut it has value:\n\""RFS_PF"\"",
            index,
            RFS_PA(ast_string_literal_get_str(expected->value.value.ast)),
            RFS_PA(ast_string_literal_get_str(got->value.value.ast))
        );
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
        test_tokens_cmp(&tokens[i], t, i, l->file, filename, line);
        i ++;
    }
}
