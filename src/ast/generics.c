#include <ast/generics.h>

#include <rfbase/utils/sanity.h>
#include <rfbase/string/core.h>

#include <ast/ast.h>
#include <ast/identifier.h>

struct ast_node *ast_genrtype_create(struct ast_node *type, struct ast_node *id)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_TYPE, ast_node_startmark(type),
                                ast_node_endmark(id));
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }

    ast_node_add_child(ret, type);
    ret->genrtype.type = type;
    ast_node_add_child(ret, id);
    ret->genrtype.id = id;
    return ret;
}

i_INLINE_INS const struct RFstring *ast_genrtype_type_str(struct ast_node *n);
i_INLINE_INS const struct RFstring *ast_genrtype_id_str(struct ast_node *n);

struct ast_node *ast_genrdecl_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_DECLARATION, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }
    return ret;
}

struct ast_node *ast_genrdecl_string_is_genr(struct ast_node *n,
                                             const struct RFstring *id)
{

    AST_NODE_ASSERT_TYPE(n, AST_GENERIC_DECLARATION);
    struct ast_node **child;
    darray_foreach(child, n->children) {
        if (rf_string_equal(id, ast_genrtype_id_str(*child))) {
            return *child;
        }
    }
    return NULL;
}

struct ast_node *ast_genrattr_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end)
{
    struct ast_node *ret;

    ret = ast_node_create_marks(AST_GENERIC_ATTRIBUTE, start, end);
    if (!ret) {
        RF_ERRNOMEM();
        return NULL;
    }
    return ret;
}

