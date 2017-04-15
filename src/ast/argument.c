#include <ast/argument.h>

#include <rfbase/utils/memory.h>

struct ast_argument *ast_argument_create(struct RFstring *name, struct type *t)
{
    struct ast_argument *ret;
    RF_MALLOC(ret, sizeof(struct ast_argument), return NULL);

    ret->name = name;
    ret->type = t;
    return ret;
}


void ast_argument_destroy(struct ast_argument *a)
{
    free(a);
}
