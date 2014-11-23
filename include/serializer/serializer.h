#ifndef LFR_SERIALIZER_H
#define LFR_SERIALIZER_H

#include <stdbool.h>
#include <stdio.h>


struct ast_node;
struct analyzer;

struct serializer {
    struct ast_node *root;
};


struct serializer *serializer_create();

bool serializer_init(struct serializer *sr);

void serializer_destroy(struct serializer *sr);

bool serializer_serialize_file(struct serializer *sr, struct analyzer *a);

#endif
