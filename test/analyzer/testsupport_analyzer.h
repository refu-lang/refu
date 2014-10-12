#ifndef LFR_TESTSUPPORT_ANALYZER_H
#define LFR_TESTSUPPORT_ANALYZER_H

#include <stdbool.h>
#include <inplocation.h>
#include <Data_Structures/darray.h>

struct ast_node;

struct analyzer_testdriver {
    //TODO: Think, if we will need this additional driver for analyzer tests
    int something;
};
struct analyzer_testdriver *get_analyzer_testdriver();

void setup_analyzer_tests();
void teardown_analyzer_tests();

#endif
