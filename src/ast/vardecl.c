#include <ast/vardecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_vardecl_create(const struct inplocation_mark *start,
                                    const struct inplocation_mark *end,
                                    struct ast_node *leaf)
{
    struct ast_node *ret;
    RF_ASSERT(
        leaf->type == AST_TYPE_LEAF,
        "Illegal ast node type \""RFS_PF"\"in vardecl creation",
        RFS_PA(leaf)
    );

    ret = ast_node_create_marks(AST_VARIABLE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, leaf, vardecl.leaf);
    return ret;
}

i_INLINE_INS struct ast_node *ast_vardecl_desc_get(const struct ast_node *n);
