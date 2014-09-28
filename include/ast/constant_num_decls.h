#ifndef LFR_AST_CONSTANT_NUMBER_DECLS_H
#define LFR_AST_CONSTANT_NUMBER_DECLS_H

#include <stdint.h>

enum constant_type {
    CONSTANT_NUMBER_FLOAT,
    CONSTANT_NUMBER_INTEGER,
};

struct ast_constantnum {
    enum constant_type type;
    union {
        uint64_t integer;
        double floating;
    } value;
};

#endif
