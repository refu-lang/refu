#ifndef LFR_TESTSUPPORT_END_TO_END_H
#define LFR_TESTSUPPORT_END_TO_END_H

#include <compiler.h>

struct RFstring;

struct end_to_end_driver {
    struct compiler compiler;
    struct RFstring *file_name;
};

struct end_to_end_driver *get_end_to_end_driver();

void setup_end_to_end_tests();
void teardown_end_to_end_tests();

bool end_to_end_driver_create_file(struct end_to_end_driver *d,
                                   const char *file_name,
                                   const struct RFstring *s);

bool end_to_end_driver_compile(struct end_to_end_driver *d, char *args);

bool end_to_end_driver_run(struct end_to_end_driver *d, int *ret_value);



#endif
