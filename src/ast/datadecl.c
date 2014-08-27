#include <ast/datadecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>



struct ast_node *ast_datadecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name,
                                     struct ast_node *desc)
{
    struct ast_node *ret;
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(desc->type == AST_DATA_DESCRIPTION);

    ret = ast_node_create(AST_DATA_DECLARATION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->datadecl.name = name;
    ret->datadecl.desc = desc;
    return ret;
}

void ast_datadecl_destroy(struct ast_node *n)
{
    ast_node_destroy(n->datadecl.name);
    ast_node_destroy(n->datadecl.desc);
}

struct RFstring *ast_datadecl_name_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_DATA_DECLARATION);

    return ast_identifier_str(n->datadecl.name);
}

void ast_datadecl_print(struct ast_node *n, int depth, const char *description)
{
    ast_print(n->datadecl.name, depth + 1, "name");
    ast_print(n->datadecl.desc, depth + 1, NULL);

}
