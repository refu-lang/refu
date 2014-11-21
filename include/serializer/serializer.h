#ifndef LFR_SERIALIZER_H
#define LFR_SERIALIZER_H

#include <stdbool.h>
#include <stdio.h>

struct serializer {
    struct ast_node *root;
};


struct serializer *serializer_create(struct ast_node *root);

bool serializer_init(struct serializer *sr, struct ast_node *root);

bool serializer_to_file(struct serializer *sr, FILE *f);

#endif
