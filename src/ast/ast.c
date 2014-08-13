#include <ast/ast.h>

#include <Utils/sanity.h>
#include <Utils/build_assert.h>
#include <RFmemory.h>


static const struct RFstring ast_type_strings[] = {
    RF_STRING_STATIC_INIT("root"),
    RF_STRING_STATIC_INIT("block"),
    RF_STRING_STATIC_INIT("variable declaration"),
    RF_STRING_STATIC_INIT("data declaration"),
    RF_STRING_STATIC_INIT("string literal"),
    RF_STRING_STATIC_INIT("identifier")
};

#define AST_NODE_IS_LEAF(node_) ((node_)->type >= AST_STRING_LITERAL)

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

    /* nodes that will only have children should initialize the list */
    switch (ret->type) {
    case AST_ROOT:
    case AST_BLOCK:
        rf_ilist_head_init(&ret->children);
        break;
    }
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
    switch (n->type) {
    case AST_ROOT:
    case AST_BLOCK:
        rf_ilist_for_each_safe(&n->children, child, tmp, lh) {
            ast_node_destroy(child);
        }
        break;
    case AST_VARIABLE_DECLARATION:
        ast_node_destroy(n->vardecl.name);
        ast_node_destroy(n->vardecl.type);
        break;
     }

    free(n);
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add(&parent->children, &child->lh);
}

const struct RFstring *ast_node_str(struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT);
    return &ast_type_strings[n->type];
}


void ast_print(struct ast_node *n, int depth)
{
    struct ast_node *c;
    struct RFilist_head *list = NULL;

    int i = 0;

    for (i = 0; i < depth; i++) {
        if (i == depth - 1) {
            if (AST_NODE_IS_LEAF(n)) {
                printf("|---->");
            } else {
                printf("|---+>");

            }
        } else {
            printf("    ", i);
        }
    }

    printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n)));
    switch(n->type) {
    case AST_ROOT:
    case AST_BLOCK:
        rf_ilist_for_each(&n->children, c, lh) {
            ast_print(c, depth + 1);
        }
        break;
    case AST_VARIABLE_DECLARATION:
        ast_print(n->vardecl.name, depth + 1);
        ast_print(n->vardecl.type, depth + 1);
        break;
    default:
        break;
    }
}


void ast_vardecl_init(struct ast_node *n,
                      struct ast_node *name,
                      struct ast_node *type)
{
    RF_ASSERT(n->type == AST_VARIABLE_DECLARATION);
    RF_ASSERT(name->type == AST_IDENTIFIER);
    RF_ASSERT(type->type == AST_IDENTIFIER);

    n->vardecl.name = name;
    n->vardecl.type = type;
}

void ast_datadecl_init(struct ast_node *n, struct ast_node *name)
{
    RF_ASSERT(n->type == AST_DATA_DECLARATION);
    RF_ASSERT(name->type == AST_IDENTIFIER);

    n->datadecl.name = name;
    rf_ilist_head_init(&n->datadecl.members);
}

void ast_datadecl_add_child(struct ast_node *n, struct ast_node *c)
{
    RF_ASSERT(n->type == AST_DATA_DECLARATION);
    RF_ASSERT(c->type == AST_VARIABLE_DECLARATION);

    rf_ilist_add(&n->datadecl.members, &c->lh);
}
