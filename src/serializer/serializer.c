#include <serializer/serializer.h>

#include <Utils/memory.h>

bool serializer_init(struct serializer *sr, struct ast_node *root)
{
    sr->root = root;
    return true;
}

struct serializer *serializer_create(struct ast_node *root)
{
    struct serializer *sr;
    RF_MALLOC(sr, sizeof(*sr), return NULL);

    if (!serializer_init(sr, root)) {
        free(sr);
        return NULL;
    }

    return sr;
}
