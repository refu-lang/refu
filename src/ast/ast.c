#include <ast/ast.h>
#include <RFmemory.h>

#define AST_NODE_NOT_LEAF(node_) ((node_)->type < AST_LEAVES)

struct ast_node *ast_node_create(enum ast_type type,
                                 struct parser_file *f,
                                 char *sp, char *ep)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), NULL);

    ret->type = type;
    if (!ast_location_init(&ret->location, f, sp, ep)) {
        return NULL;
    }
    rf_ilist_head_init(&ret->children);

    return ret;    
}

//will probably go away if not used
struct ast_node *ast_node_create_fromloc(enum ast_type type,
                                         struct ast_location *loc)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), NULL);

    ret->type = type;
    ast_location_copy(&ret->location, loc);
    rf_ilist_head_init(&ret->children);

    return ret;
}

void ast_node_destroy(struct ast_node *n)
{
    struct ast_node *child;
    struct ast_node *tmp;
    if (AST_NODE_NOT_LEAF(n)) {
        rf_ilist_for_each_safe(&n->children, child, tmp, lh) {
            ast_node_destroy(child);
        }
     }

    //TODO: specific node handling
    /* switch (type) { */
    /* case AST_ROOT: */
    /* } */
    free(n);
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add(&parent->children, &child->lh);
    parent->children_num ++;
}
