#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Suite *lexer_suite_create(void);
Suite *frontend_input_suite_create(void);
Suite *parser_typedesc_suite_create(void);
Suite *parser_generics_suite_create(void);
Suite *parser_function_suite_create(void);
Suite *parser_typeclass_suite_create(void);
Suite *parser_operators_suite_create(void);
Suite *parser_block_suite_create(void);
Suite *parser_forexpr_suite_create(void);
Suite *parser_ifexpr_suite_create(void);
Suite *parser_matchexpr_suite_create(void);
Suite *parser_modules_suite_create(void);
Suite *parser_misc_suite_create(void);

Suite *analyzer_symboltable_suite_create(void);
Suite *analyzer_typecheck_suite_create(void);
Suite *analyzer_typecheck_array_suite_create(void);
Suite *analyzer_typecheck_conversion_suite_create(void);
Suite *analyzer_typecheck_forexpr_suite_create(void);
Suite *analyzer_typecheck_functions_suite_create(void);
Suite *analyzer_typecheck_matchexpr_suite_create(void);
Suite *analyzer_typecheck_operators_suite_create(void);
Suite *analyzer_typecheck_typeclasses_suite_create(void);
Suite *analyzer_modules_suite_create(void);

Suite *types_suite_create(void);
Suite *type_set_suite_create(void);

Suite *rir_finalized_ast_suite_create(void);
Suite *rir_creation_simple_suite_create(void);
Suite *rir_parsing_suite_create(void);
Suite *rir_end_to_end_suite_create(void);
Suite *rir_misctest_suite_create(void);

Suite *ownership_suite_create(void);

Suite *end_to_end_basic_suite_create(void);
Suite *end_to_end_module_suite_create(void);

static const char *SILENT = "CK_SILENT";
static const char *MINIMAL = "CK_MINIMAL";
static const char *NORMAL = "CK_NORMAL";
static const char *VERBOSE = "CK_VERBOSE";


static enum print_output choose_print_output(char* arg)
{
    if (strcmp(arg, SILENT) == 0) {
        return CK_SILENT;
    } else if (strcmp(arg, MINIMAL) == 0) {
        return CK_MINIMAL;
    } else if (strcmp(arg, NORMAL) == 0) {
        return CK_NORMAL;
    } else if (strcmp(arg, VERBOSE) == 0) {
        return CK_VERBOSE;
    } else {
        //should never happen but let's just go verbose
        return CK_VERBOSE;
    }
}

static enum fork_status choose_fork_status(char* arg)
{
    enum fork_status type = CK_FORK;
    if (strcmp(arg, "False") == 0) {
        type = CK_NOFORK;
    }
    return type;
}

int main(int argc, char **argv)
{
    int number_failed;
    enum print_output print_type;
    enum fork_status fork_type;
    /* default values */
    print_type = CK_VERBOSE;
    fork_type = CK_FORK;

    if (argc >= 2) {
        print_type = choose_print_output(argv[1]);
    }
    if (argc >= 3) {
        fork_type = choose_fork_status(argv[2]);
    }

    printf("\n\n=== Running refulang tests ===\n");
    SRunner *sr = srunner_create(lexer_suite_create());
    srunner_add_suite(sr, frontend_input_suite_create());
    srunner_add_suite(sr, parser_typedesc_suite_create());
    srunner_add_suite(sr, parser_generics_suite_create());
    srunner_add_suite(sr, parser_function_suite_create());
    srunner_add_suite(sr, parser_typeclass_suite_create());
    srunner_add_suite(sr, parser_operators_suite_create());
    srunner_add_suite(sr, parser_block_suite_create());
    srunner_add_suite(sr, parser_ifexpr_suite_create());
    srunner_add_suite(sr, parser_forexpr_suite_create());
    srunner_add_suite(sr, parser_matchexpr_suite_create());
    srunner_add_suite(sr, parser_modules_suite_create());
    srunner_add_suite(sr, parser_misc_suite_create());

    srunner_add_suite(sr, analyzer_symboltable_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_array_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_conversion_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_forexpr_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_functions_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_matchexpr_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_operators_suite_create());
    srunner_add_suite(sr, analyzer_typecheck_typeclasses_suite_create());
    srunner_add_suite(sr, analyzer_modules_suite_create());

    srunner_add_suite(sr, types_suite_create());
    srunner_add_suite(sr, type_set_suite_create());

    srunner_add_suite(sr, rir_finalized_ast_suite_create());
    srunner_add_suite(sr, rir_creation_simple_suite_create());
    srunner_add_suite(sr, rir_parsing_suite_create());
    srunner_add_suite(sr, rir_end_to_end_suite_create());
    srunner_add_suite(sr, rir_misctest_suite_create());
    
    srunner_add_suite(sr, ownership_suite_create());

    srunner_add_suite(sr, end_to_end_basic_suite_create());
    srunner_add_suite(sr, end_to_end_module_suite_create());

    srunner_set_fork_status (sr, fork_type);
    srunner_run_all(sr, print_type);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    /* close standard streams to silence valgrind */
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
