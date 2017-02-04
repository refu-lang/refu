#ifndef LFR_AST_ITERABLE_DECLS_H
#define LFR_AST_ITERABLE_DECLS_H

struct ast_node;

enum iterable_type {
    ITERABLE_COLLECTION = 0,
    ITERABLE_RANGE = 1,
};

struct int_range {
    int start;
    int step;
    int end;
};

struct ast_iterable {
    enum iterable_type type;
    union {
        struct ast_node *identifier;
        struct int_range range;
    };
};

#endif
