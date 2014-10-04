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
    RF_ASSERT(type->type == AST_IDENTIFIER);

    ret = ast_node_create_marks(AST_VARIABLE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ret->vardecl.name = name;
    ret->vardecl.type = type;    
    
    return ret;
}

struct RFstring *ast_vardecl_name_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_VARIABLE_DECLARATION);

    return ast_identifier_str(n->vardecl.name);
}

struct RFstring *ast_vardecl_type_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_VARIABLE_DECLARATION);

    return ast_identifier_str(n->vardecl.type);
}
