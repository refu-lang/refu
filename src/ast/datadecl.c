#include <ast/datadecl.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <Utils/sanity.h>

i_INLINE_INS const struct RFstring *dataop_type_str(enum dataop_type type);

struct ast_node *ast_dataop_create(struct parser_file *f,
                                   char *sp,
                                   char *ep,
                                   enum dataop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right)
{
    struct ast_node *ret;
    RF_ASSERT(left->type == AST_DATA_DESCRIPTION);
    RF_ASSERT(right->type == AST_DATA_DESCRIPTION);
    ret = ast_node_create(AST_DATA_OPERATOR, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    ret->dataop.type = type;
    ret->dataop.left = left;
    ret->dataop.right = right;
    return ret;
}
void ast_dataop_destroy(struct ast_node *n)
{
    ast_node_destroy(n->dataop.left);
    ast_node_destroy(n->dataop.right);
}


void ast_dataop_print(struct ast_node *n, int depth, const char *description)
{
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("DATAOP: "RF_STR_PF_FMT"\n",
           RF_STR_PF_ARG(dataop_type_str(n->dataop.type)));
    ast_print(n->dataop.left, depth + 1, "left");
    ast_print(n->dataop.right, depth + 1, "right");
}


struct ast_node *ast_datadesc_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *id,
                                     bool dataop)
{
    struct ast_node *ret;

    ret = ast_node_create(AST_DATA_DESCRIPTION, f, sp, ep);
    if (!ret) {
        //TODO: memory error
        return NULL;
    }

    if (dataop) {
        RF_ASSERT(id->type == AST_DATA_OPERATOR);
        ret->datadesc.is_dataop = true;
        ret->datadesc.dataop = id;
    } else {
        RF_ASSERT(id->type == AST_IDENTIFIER);
        ret->datadesc.is_dataop = false;
        ret->datadesc.left = id;
        ret->datadesc.right = NULL;
    }
    return ret;
}

void ast_datadesc_destroy(struct ast_node *n)
{
    if (n->datadesc.is_dataop) {
        ast_dataop_destroy(n->datadesc.dataop);
    } else {
        ast_node_destroy(n->datadesc.left);
        if (n->datadesc.right) {
            ast_node_destroy(n->datadesc.right);
        }
    }
}


void ast_datadesc_set_right(struct ast_node *n, struct ast_node *r)
{
    RF_ASSERT(n->type == AST_DATA_DESCRIPTION);
    RF_ASSERT(r->type == AST_DATA_DESCRIPTION);
    n->datadesc.right = r;
}

void ast_datadesc_set_dop(struct ast_node *n, struct ast_node *dop)
{
    RF_ASSERT(n->type == AST_DATA_DESCRIPTION);
    RF_ASSERT(dop->type == AST_DATA_OPERATOR);
    n->datadesc.dataop = dop;
}

void ast_datadesc_print(struct ast_node *n, int depth, const char *description)
{
    if (n->datadesc.is_dataop) {
        ast_print(n->datadesc.dataop, depth + 1, NULL);
    } else {
        ast_print(n->datadesc.left, depth + 1, NULL);
        if (n->datadesc.right) {
            ast_print(n->datadesc.right, depth + 1, NULL);
        }
    }
}





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
