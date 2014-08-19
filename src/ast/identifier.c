#include <ast/identifier.h>

#include <ast/ast.h>
#include <Utils/sanity.h>

struct ast_node *ast_identifier_create(struct parser_file *file,
                                       char *sp, char *ep)
{
    struct ast_node *ret;
    ret = ast_node_create(AST_IDENTIFIER,
                          file, sp, ep);

    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->identifier, sp, ep - sp);

    return ret;
}

struct RFstring *ast_identifier_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_IDENTIFIER);
    return &n->identifier;
}

void ast_identifier_print(struct ast_node *n, int depth)
{
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("Value: \""RF_STR_PF_FMT"\"\n", RF_STR_PF_ARG(&n->identifier));
}
