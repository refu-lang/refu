#include <serializer/serializer.h>

#include <Utils/memory.h>

#include <analyzer/analyzer.h>

bool serializer_init(struct serializer *sr)
{
    // TODO: can delete if nothing needs initializing
    return true;
}

struct serializer *serializer_create()
{
    struct serializer *sr;
    RF_MALLOC(sr, sizeof(*sr), return NULL);

    if (!serializer_init(sr)) {
        free(sr);
        return NULL;
    }

    return sr;
}

void serializer_destroy(struct serializer *sr)
{
    free(sr);
}

bool serializer_serialize_file(struct serializer *sr, struct analyzer *a)
{
    //TODO
    return true;
}