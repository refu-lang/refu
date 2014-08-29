#include <ast/ast.h>

#include <Utils/sanity.h>
#include <Utils/build_assert.h>
#include <RFmemory.h>


static const struct RFstring ast_type_strings[] = {
    RF_STRING_STATIC_INIT("root"),
    RF_STRING_STATIC_INIT("block"),
    RF_STRING_STATIC_INIT("variable declaration"),
    RF_STRING_STATIC_INIT("type declaration"),
    RF_STRING_STATIC_INIT("type operator"),
    RF_STRING_STATIC_INIT("type description"),
    RF_STRING_STATIC_INIT("generic declaration"),
    RF_STRING_STATIC_INIT("generic type"),
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
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ret->type = type;
    if (!ast_location_init(&ret->location, f, sp, ep)) {
        return NULL;
    }
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
    case AST_TYPE_DECLARATION:
    case AST_TYPE_OPERATOR:
    case AST_TYPE_DESCRIPTION:
        /* Only delete children list */
    case AST_IDENTIFIER:
        /* no need to free, is a shallow pointer to the parsed file's string */
        break;
    case AST_VARIABLE_DECLARATION:
        ast_node_destroy(n->vardecl.name);
        ast_node_destroy(n->vardecl.type);
        break;
    case AST_GENERIC_DECLARATION:
        ast_genrdecl_destroy(n);
        break;
    case AST_GENERIC_TYPE:
        ast_genrtype_destroy(n);
        break;
    case AST_FUNCTION_DECLARATION:
        ast_fndecl_destroy(n);
        break;
    default:
        RF_ASSERT(0);
        break;
     }

    rf_ilist_for_each_safe(&n->children, child, tmp, lh) {
        ast_node_destroy(child);
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
    rf_ilist_add_tail(&parent->children, &child->lh);
}


i_INLINE_INS char *ast_node_startsp(struct ast_node *n);
i_INLINE_INS char *ast_node_endsp(struct ast_node *n);
const struct RFstring *ast_node_str(struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT
    );
    return &ast_type_strings[n->type];
}

static void ast_print_prelude(struct ast_node *n, int depth, const char *desc)
{
    int i = 0;

    if (depth != 0) {
        if (desc) {
            printf("%s", desc);
            printf("%*s", (depth * AST_PRINT_DEPTHMUL) - strlen(desc), " ");
            printf("|----> "RF_STR_PF_FMT" "AST_LOCATION_FMT2"\n",
                   RF_STR_PF_ARG(ast_node_str(n)),
                   AST_LOCATION_ARG2(&n->location));
        } else {
            printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
            printf("|----> "RF_STR_PF_FMT" "AST_LOCATION_FMT2"\n",
                   RF_STR_PF_ARG(ast_node_str(n)),
                   AST_LOCATION_ARG2(&n->location));
        }
    } else {
        printf("%*.*s "AST_LOCATION_FMT2"\n",
               depth * AST_PRINT_DEPTHMUL,
               RF_STR_PF_ARG(ast_node_str(n)),
               AST_LOCATION_ARG2(&n->location));
    }
}

void ast_print(struct ast_node *n, int depth)
{
    struct ast_node *c;
    struct RFilist_head *list = NULL;

    ast_print_prelude(n, depth, "");

    switch(n->type) {
    case AST_IDENTIFIER:
        ast_identifier_print(n, depth + 1);
        break;
    default:
        printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n)));
        rf_ilist_for_each(&n->children, c, lh) {
            ast_print(c, depth + 1);
        }
        break;
    }
}
