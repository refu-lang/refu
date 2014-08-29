#include <ast/typedecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>



struct ast_node *ast_typedecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name,
                                     struct ast_node *desc)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(desc->type == AST_TYPE_DESCRIPTION);

    ret = ast_node_create(AST_TYPE_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ast_node_add_child(ret, name);
    ret->typedecl.name = name;
    ast_node_add_child(ret, desc);
    ret->typedecl.desc = desc;
    return ret;
}

struct RFstring *ast_typedecl_name_str(struct ast_typedecl *t)
{
    return ast_identifier_str(t->name);
}
