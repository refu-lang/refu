#include <ast/vardecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

struct ast_node *ast_vardecl_create(struct inplocation_mark *start,
                                    struct inplocation_mark *end,
                                    struct ast_node *desc)
{
    struct ast_node *ret;
    RF_ASSERT(desc->type == AST_TYPE_DESCRIPTION ||
              desc->type == AST_TYPE_OPERATOR,
              "Illegal ast node type \""RF_STR_PF_FMT"\"in vardecl creation",
              RF_STR_PF_ARG(desc));

    ret = ast_node_create_marks(AST_VARIABLE_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_register_child(ret, desc, vardecl.desc);
    return ret;
}

i_INLINE_INS struct ast_node *ast_vardecl_desc_get(struct ast_node *n);
