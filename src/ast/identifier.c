#include <ast/identifier.h>

#include <ast/ast.h>
#include <Utils/sanity.h>


struct ast_node *ast_identifier_create(struct inplocation *loc)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_IDENTIFIER, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->identifier.string, loc->start.p,
                           loc->end.p - loc->start.p + 1);

    return ret;
}

void ast_identifier_print(struct ast_node *n, int depth)
{
    RF_ASSERT(n->type == AST_IDENTIFIER);
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("Value: \""RF_STR_PF_FMT"\"\n",
               RF_STR_PF_ARG(&n->identifier.string));
}

const struct RFstring *ast_identifier_str(const struct ast_node *n)
{
    RF_ASSERT(n->type == AST_IDENTIFIER);
    return &n->identifier.string;
}


struct ast_node *ast_xidentifier_create(struct inplocation_mark *start,
                                        struct inplocation_mark *end,
                                        struct ast_node *id,
                                        bool is_constant,
                                        struct ast_node *genr)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_XIDENTIFIER, start, end);
    if (!ret) {
        return NULL;
    }

    ast_node_add_child(ret, id);
    ret->xidentifier.is_constant = is_constant;
    ret->xidentifier.id = id;
    ret->xidentifier.genr = genr;
    if (genr) {
        ast_node_add_child(ret, genr);
    }

    return ret;
}
