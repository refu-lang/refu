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
    RF_STRING_STATIC_INIT("typeclass declaration"),
    RF_STRING_STATIC_INIT("typeclass instance"),
    RF_STRING_STATIC_INIT("generic declaration"),
    RF_STRING_STATIC_INIT("generic type"),
    RF_STRING_STATIC_INIT("generic attribute"),
    RF_STRING_STATIC_INIT("function declaration"),
    RF_STRING_STATIC_INIT("function implementation"),
    RF_STRING_STATIC_INIT("function call"),
    RF_STRING_STATIC_INIT("array reference"),
    RF_STRING_STATIC_INIT("annotated identifier"),
    RF_STRING_STATIC_INIT("binary operator"),
    RF_STRING_STATIC_INIT("unary operator"),
    RF_STRING_STATIC_INIT("string literal"),
    RF_STRING_STATIC_INIT("identifier"),
    RF_STRING_STATIC_INIT("constant number"),
};

#define AST_NODE_IS_LEAF(node_) ((node_)->type >= AST_STRING_LITERAL)

void ast_node_init(struct ast_node * n, enum ast_type type)
{
    n->type = type;
    rf_ilist_head_init(&n->children);
}

struct ast_node *ast_node_create(enum ast_type type)
{
   struct ast_node *ret;
   RF_MALLOC(ret, sizeof(struct ast_node), return NULL);
   ast_node_init(ret, type);
   return ret;
}

struct ast_node *ast_node_create_loc(enum ast_type type,
                                     struct inplocation *loc)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    inplocation_copy(&ret->location, loc);

    return ret;
}

struct ast_node *ast_node_create_marks(enum ast_type type,
                                       struct inplocation_mark *start,
                                       struct inplocation_mark *end)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    inplocation_init_marks(&ret->location, start, end);

    return ret;
}

struct ast_node *ast_node_create_ptrs(enum ast_type type,
                                      struct inpfile *f,
                                      char *sp, char *ep)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    if (!inplocation_init(&ret->location, f, sp, ep)) {
        return NULL;
    }

    return ret;
}


void ast_node_destroy(struct ast_node *n)
{
    struct ast_node *child;
    struct ast_node *tmp;

    /* type specific destruction */
    //TODO: if ever needing destruction specific to an ast type
    //      perform it with a switch() here.

    rf_ilist_for_each_safe(&n->children, child, tmp, lh) {
        ast_node_destroy(child);
    }

    free(n);
}

void ast_node_set_start(struct ast_node *n, struct inplocation_mark *start)
{
    inplocation_set_start(&n->location, start);
}

void ast_node_set_end(struct ast_node *n, struct inplocation_mark *end)
{
    inplocation_set_end(&n->location, end);
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add_tail(&parent->children, &child->lh);
}


i_INLINE_INS char *ast_node_startsp(struct ast_node *n);
i_INLINE_INS char *ast_node_endsp(struct ast_node *n);
i_INLINE_INS struct inplocation_mark *ast_node_startmark(struct ast_node *n);
i_INLINE_INS struct inplocation_mark *ast_node_endmark(struct ast_node *n);

const struct RFstring *ast_node_str(struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT
    );
    return &ast_type_strings[n->type];
}

static void ast_print_prelude(struct ast_node *n, struct inpfile *f,
                              int depth, const char *desc)
{
    if (depth != 0) {
        if (desc) {
            printf("%s", desc);
            printf("%*s",
                   (int)((depth * AST_PRINT_DEPTHMUL) - strlen(desc)),
                   " ");
            printf("|----> "RF_STR_PF_FMT" "INPLOCATION_FMT2"\n",
                   RF_STR_PF_ARG(ast_node_str(n)),
                   INPLOCATION_ARG2(f, &n->location));
        } else {
            printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
            printf("|----> "RF_STR_PF_FMT" "INPLOCATION_FMT2"\n",
                   RF_STR_PF_ARG(ast_node_str(n)),
                   INPLOCATION_ARG2(f, &n->location));
        }
    } else {
        printf("%*.*s "INPLOCATION_FMT2"\n",
               depth * AST_PRINT_DEPTHMUL,
               RF_STR_PF_ARG(ast_node_str(n)),
               INPLOCATION_ARG2(f, &n->location));
    }
}

void ast_print(struct ast_node *n, struct inpfile *f, int depth)
{
    struct ast_node *c;
    ast_print_prelude(n, f, depth, "");

    switch(n->type) {
    case AST_IDENTIFIER:
        ast_identifier_print(n, depth + 1);
        break;
    default:
        printf(RF_STR_PF_FMT"\n", RF_STR_PF_ARG(ast_node_str(n)));
        rf_ilist_for_each(&n->children, c, lh) {
            ast_print(c, f, depth + 1);
        }
        break;
    }
}
