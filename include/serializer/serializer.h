#ifndef LFR_SERIALIZER_H
#define LFR_SERIALIZER_H

#include <stdbool.h>
#include <stdio.h>

enum serializer_rc {
    SERC_SUCCESS_CONTINUE = 0,
    SERC_SUCCESS_EXIT,
    SERC_FAILURE,
    
};

struct ast_node;
struct analyzer;

/**
 * The serializer deals with data exporting and serialization (if needed)
 *after the end of a succesful analysis.
 */
struct serializer {
    //! A non-owned pointer to the AST
    struct ast_node *root;
    //! A non-owned pointer to object holding the compiler arguments
    struct compiler_args *args;
};


struct serializer *serializer_create(struct compiler_args *args);

bool serializer_init(struct serializer *sr, struct compiler_args *args);

void serializer_destroy(struct serializer *sr);

bool serializer_process(struct serializer *sr, const struct ast_node *root);


#endif
