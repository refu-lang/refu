#include <ast/vardecl.h>

#include <ast/ast.h>
#include <Utils/sanity.h>

struct ast_node *ast_vardecl_create(struct parser_file *f,
                                    char *sp,
                                    char *ep, 
                                    struct ast_node *name,
                                    struct ast_node *type)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(type->type == AST_IDENTIFIER);

    ret = ast_node_create(AST_VARIABLE_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->vardecl.name = name;
    ret->vardecl.type = type;    
    
    return ret;
}
