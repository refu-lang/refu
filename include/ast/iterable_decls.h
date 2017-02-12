#ifndef LFR_AST_ITERABLE_DECLS_H
#define LFR_AST_ITERABLE_DECLS_H

#include <stdint.h>

struct ast_node;

enum iterable_type {
    ITERABLE_COLLECTION = 0,
    ITERABLE_RANGE = 1,
};

struct int_range {
    int64_t start;
    int64_t step;
    int64_t end;
    struct ast_node *start_node;
    struct ast_node *step_node;
    struct ast_node *end_node;
};

struct ast_iterable {
    enum iterable_type type;
    union {
        struct ast_node *identifier;
        struct int_range range;
    };
};

#endif
