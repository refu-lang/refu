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

    ret->typedecl.name = name;
    ret->typedecl.desc = desc;
    return ret;
}

void ast_typedecl_destroy(struct ast_node *n)
{
    ast_node_destroy(n->typedecl.name);
    ast_node_destroy(n->typedecl.desc);
}

struct RFstring *ast_typedecl_name_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_TYPE_DECLARATION);

    return ast_identifier_str(n->typedecl.name);
}

void ast_typedecl_print(struct ast_node *n, int depth, const char *description)
{
    ast_print(n->typedecl.name, depth + 1, "name");
    ast_print(n->typedecl.desc, depth + 1, NULL);

}
