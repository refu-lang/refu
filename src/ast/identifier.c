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
    RF_STRING_SHALLOW_INIT(&ret->identifier.string, sp, ep - sp + 1);

    return ret;
}

void ast_identifier_print(struct ast_node *n, int depth)
{
    RF_ASSERT(n->type == AST_IDENTIFIER);
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("Value: \""RF_STR_PF_FMT"\"\n",
               RF_STR_PF_ARG(&n->identifier.string));
}

struct RFstring *ast_identifier_str(struct ast_node *n)
{
    RF_ASSERT(n->type == AST_IDENTIFIER);
    return &n->identifier.string;
}


struct ast_node *ast_xidentifier_create(struct parser_file *f,
                                        char *sp, char *ep,
                                        struct ast_node *id,
                                        bool is_constant,
                                        struct ast_node *genr)
{
    struct ast_node *ret;
    ret = ast_node_create(AST_XIDENTIFIER, f, sp, ep);
    if (!ret) {
        return NULL;
    }

    ast_node_add_child(ret, id);
    ret->xidentifier.is_constant = is_constant;
    ret->xidentifier.id = id;
    ret->xidentifier.genr = genr;

    return ret;
}
