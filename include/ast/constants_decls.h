#ifndef LFR_AST_CONSTANTS_DECLS_H
#define LFR_AST_CONSTANTS_DECLS_H

#include <stdint.h>
#include <stdbool.h>

enum constant_type {
    CONSTANT_NUMBER_FLOAT,
    CONSTANT_NUMBER_INTEGER,
    CONSTANT_BOOLEAN,
};

struct ast_constant {
    enum constant_type type;
    union {
        int64_t integer;
        double floating;
        bool boolean;
    } value;
};

#endif
