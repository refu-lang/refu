#include <ast/identifier.h>

#include <ast/ast.h>

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
