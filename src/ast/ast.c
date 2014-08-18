#include <ast/ast.h>

#include <Utils/sanity.h>
#include <Utils/build_assert.h>
#include <RFmemory.h>


static const struct RFstring ast_type_strings[] = {
    RF_STRING_STATIC_INIT("root"),
    RF_STRING_STATIC_INIT("block"),
    RF_STRING_STATIC_INIT("variable declaration"),
    RF_STRING_STATIC_INIT("data declaration"),
    RF_STRING_STATIC_INIT("generic declaration"),
    RF_STRING_STATIC_INIT("function declaration"),
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
    case AST_IDENTIFIER:
        /* no need to free, is a shallow pointer to the parsed file's string */
        break;
    case AST_VARIABLE_DECLARATION:
        ast_node_destroy(n->vardecl.name);
        ast_node_destroy(n->vardecl.type);
        break;
    case AST_DATA_DECLARATION:
        ast_datadecl_destroy(n);
        break;
    case AST_GENERIC_DECLARATION:
        ast_genrdecl_destroy(n);
        break;
    case AST_FUNCTION_DECLARATION:
        ast_fndecl_destroy(n);
        break;
    default:
        RF_ASSERT(0);
        break;
     }


    free(n);
}

bool ast_node_set_end(struct ast_node *n, char *end)
{
    return ast_location_set_end(&n->location, end);
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add(&parent->children, &child->lh);
}

const struct RFstring *ast_node_str(struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT
    );
    return &ast_type_strings[n->type];
}


static void ast_print_prelude(struct ast_node *n, int depth)
{
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
}

void ast_print(struct ast_node *n, int depth)
{
    struct ast_node *c;
    struct ast_genrtype *cg;
    struct RFilist_head *list = NULL;

    ast_print_prelude(n, depth);

    switch(n->type) {
    case AST_ROOT:
    case AST_BLOCK:
        printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n)));
        rf_ilist_for_each(&n->children, c, lh) {
            ast_print(c, depth + 1);
        }
        break;
    case AST_DATA_DECLARATION:
        printf("data declaration  name:\""RF_STR_PF_FMT"\"\n",
               RF_STR_PF_ARG(ast_datadecl_name_str(n)));
         rf_ilist_for_each(&n->datadecl.members, c, lh) {
            ast_print(c, depth + 1);
         }
        break;
    case AST_GENERIC_DECLARATION:
        printf("generic declaration:\n");
         rf_ilist_for_each(&n->genrdecl.members, cg, lh) {
             ast_print(cg->id, depth + 1);
         }
        break;
    case AST_FUNCTION_DECLARATION:
        printf("function declaration  name:\""RF_STR_PF_FMT"\" ",
               RF_STR_PF_ARG(ast_fndecl_name_str(n)));
        if (n->fndecl.ret) {
            printf("return :\""RF_STR_PF_FMT"\" ",
               RF_STR_PF_ARG(ast_fndecl_ret_str(n)));
        }
        if (n->fndecl.genr) {
            printf("with generics:\n");
            ast_print(n->fndecl.genr, depth + 1);
        }
        printf("with arguments: \n");

         rf_ilist_for_each(&n->fndecl.args, c, lh) {
            ast_print(c, depth + 1);
         }
        break;
    case AST_VARIABLE_DECLARATION:
        printf("variable declaration  name:\""RF_STR_PF_FMT"\""
               ", type:\"" RF_STR_PF_FMT"\"\n",
               RF_STR_PF_ARG(ast_vardecl_name_str(n)),
               RF_STR_PF_ARG(ast_vardecl_type_str(n)));
        ast_print(n->vardecl.name, depth + 1);
        ast_print(n->vardecl.type, depth + 1);
        break;
    case AST_IDENTIFIER:
        printf("identifier: "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(&n->identifier));
        break;
    default:
        printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n)));
        break;
    }
}
