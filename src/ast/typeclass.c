#include <ast/typeclass.h>

struct ast_node *ast_typeclass_create(struct inplocation_mark *start,
                                      struct inplocation_mark *end,
                                      struct ast_node *name,
                                      struct ast_node *genr)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_TYPECLASS_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }


    ast_node_register_child(ret, name, fndecl.name);
    ast_node_register_child(ret, genr, fndecl.genr);

    return ret;
}

