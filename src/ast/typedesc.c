#include <ast/typedesc.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

i_INLINE_INS const struct RFstring *typeop_type_str(enum typeop_type type);

struct ast_node *ast_typeop_create(struct parser_file *f,
                                   char *sp,
                                   char *ep,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    RF_ASSERT(left->type == AST_TYPE_DESCRIPTION);
    RF_ASSERT(right->type == AST_TYPE_DESCRIPTION);
    ret = ast_node_create(AST_TYPE_OPERATOR, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->typeop.type = type;
    ret->typeop.left = left;
    ret->typeop.right = right;
    return ret;
}
void ast_typeop_destroy(struct ast_node *n)
{
    ast_node_destroy(n->typeop.left);
    ast_node_destroy(n->typeop.right);
}


void ast_typeop_print(struct ast_node *n, int depth, const char *description)
{
    /* printf("%*s", depth * AST_PRINT_DEPTHMUL, " "); */
    /* printf("DATAOP: "RF_STR_PF_FMT"\n", */
    /*        RF_STR_PF_ARG(typeop_type_str(n->typeop.type))); */
    ast_print(n->typeop.left, depth + 1, "left");
    ast_print(n->typeop.right, depth + 1, "right");
}


struct ast_node *ast_typedesc_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *id,
                                     bool typeop)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_TYPE_DESCRIPTION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    if (typeop) {
        RF_ASSERT(id->type == AST_TYPE_OPERATOR);
        ret->typedesc.is_typeop = true;
        ret->typedesc.typeop = id;
    } else {
        RF_ASSERT(id->type == AST_IDENTIFIER);
        ret->typedesc.is_typeop = false;
        ret->typedesc.left = id;
        ret->typedesc.right = NULL;
    }
    return ret;
}

void ast_typedesc_destroy(struct ast_node *n)
{
    if (n->typedesc.is_typeop) {
        ast_typeop_destroy(n->typedesc.typeop);
    } else {
        ast_node_destroy(n->typedesc.left);
        if (n->typedesc.right) {
            ast_node_destroy(n->typedesc.right);
        }
    }
}


void ast_typedesc_set_right(struct ast_node *n, struct ast_node *r)
{
    RF_ASSERT(n->type == AST_TYPE_DESCRIPTION);
    RF_ASSERT(r->type == AST_TYPE_DESCRIPTION);
    n->typedesc.right = r;
}

void ast_typedesc_set_dop(struct ast_node *n, struct ast_node *dop)
{
    RF_ASSERT(n->type == AST_TYPE_DESCRIPTION);
    RF_ASSERT(dop->type == AST_TYPE_OPERATOR);
    n->typedesc.typeop = dop;
}

void ast_typedesc_print(struct ast_node *n, int depth, const char *description)
{
    if (n->typedesc.is_typeop) {
        ast_print(
            n->typedesc.typeop,
            depth + 1,
            rf_string_data(typeop_type_str(n->typedesc.typeop->typeop.type)));
    } else {
        ast_print(n->typedesc.left, depth + 1, NULL);
        if (n->typedesc.right) {
            ast_print(n->typedesc.right, depth + 1, NULL);
        }
    }
}
