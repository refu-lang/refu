#include <check.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Suite *parser_identifier_suite_create(void)
{
    Suite *s = suite_create("Parser - Identifier");

    TCase *p1 = tcase_create("TODO");

    suite_add_tcase(s, p1);
    return s;
}


