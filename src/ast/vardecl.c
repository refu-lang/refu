#include <ast/vardecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *name,
                                    struct ast_node *type)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(type->type == AST_TYPE_DESCRIPTION || type->type == AST_TYPE_OPERATOR);

    ret = ast_node_create_marks(AST_VARIABLE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, name, vardecl.name);
    ast_node_register_child(ret, type, vardecl.type);
    return ret;
}
