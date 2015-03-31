#include <ast/ast.h>

#include <ast/block.h>
#include <ast/function.h>
#include <ast/type.h>

#include <Utils/sanity.h>
#include <Utils/build_assert.h>
#include <RFmemory.h>

static const struct RFstring ast_type_strings[] = {
    RF_STRING_STATIC_INIT("root"),
    RF_STRING_STATIC_INIT("block"),
    RF_STRING_STATIC_INIT("variable declaration"),
    RF_STRING_STATIC_INIT("return statement"),
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
    RF_STRING_STATIC_INIT("conditional branch"),
    RF_STRING_STATIC_INIT("if expression"),
    RF_STRING_STATIC_INIT("match expression"),
    RF_STRING_STATIC_INIT("match case"),
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
    n->state = AST_NODE_STATE_CREATED;
    n->type = type;
    n->expression_type = NULL;
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
                                     const struct inplocation *loc)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    inplocation_copy(&ret->location, loc);

    return ret;
}

struct ast_node *ast_node_create_marks(enum ast_type type,
                                       const struct inplocation_mark *start,
                                       const struct inplocation_mark *end)
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

    /* type specific destruction  -- only if owned by analyzer and after */
    if (n->state >= AST_NODE_STATE_ANALYZER_PASS1) {
        switch(n->type) {
        case AST_ROOT:
            symbol_table_deinit(ast_root_symbol_table_get(n));
            break;
        case AST_BLOCK:
            symbol_table_deinit(ast_block_symbol_table_get(n));
            break;
        case AST_FUNCTION_IMPLEMENTATION:
            symbol_table_deinit(ast_fnimpl_symbol_table_get(n));
            break;
        case AST_TYPE_DECLARATION:
            symbol_table_deinit(ast_typedecl_symbol_table_get(n));
            break;
        default:
            // no type specific destruction for the rest
            break;
        }
    }

    rf_ilist_for_each_safe(&n->children, child, tmp, lh) {
        ast_node_destroy(child);
    }

    // free node unless it's a value node still at lexing/parsing phase
    if (!(n->state == AST_NODE_STATE_CREATED && ast_node_has_value(n))) {
        free(n);
    }
}

void ast_node_destroy_from_lexer(struct ast_node *n)
{
    RF_ASSERT(ast_node_has_value(n), "Requested to destroy a non value node from lexer");
    n->state = AST_NODE_STATE_AFTER_PARSING; // just make sure it's deleteable
    ast_node_destroy(n);
}

void ast_node_set_start(struct ast_node *n, const struct inplocation_mark *start)
{
    inplocation_set_start(&n->location, start);
}

void ast_node_set_end(struct ast_node *n, const struct inplocation_mark *end)
{
    inplocation_set_end(&n->location, end);
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add_tail(&parent->children, &child->lh);
}

const struct RFstring * ast_node_get_name_str(const struct ast_node *n)
{
    switch(n->type) {
    case AST_IDENTIFIER:
        return ast_identifier_str(n);
        break;
    case AST_XIDENTIFIER:
        return ast_xidentifier_str(n);
        break;
    default:
        RF_ASSERT_OR_CRITICAL(false, return NULL,
                              "Requesting identifier string from illegal"
                              "ast node type \""RF_STR_PF_FMT"\"",
                              RF_STR_PF_ARG(ast_node_str(n)));
    }
}


i_INLINE_INS struct ast_node *ast_node_get_child(struct ast_node *n,
                                                  unsigned int num);
i_INLINE_INS unsigned int ast_node_get_children_number(const struct ast_node *n);
i_INLINE_INS char *ast_node_startsp(struct ast_node *n);
i_INLINE_INS char *ast_node_endsp(struct ast_node *n);
i_INLINE_INS struct inplocation *ast_node_location(struct ast_node *n);
i_INLINE_INS struct inplocation_mark *ast_node_startmark(struct ast_node *n);
i_INLINE_INS struct inplocation_mark *ast_node_endmark(struct ast_node *n);
i_INLINE_INS enum ast_type ast_node_type(struct ast_node *n);
i_INLINE_INS bool ast_node_has_value(const struct ast_node *n);
i_INLINE_INS const struct type *ast_expression_get_type(struct ast_node *expr);

const struct RFstring *ast_nodetype_str(enum ast_type type)
{
    return &ast_type_strings[type];
}

const struct RFstring *ast_node_str(const struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT
    );
    return ast_nodetype_str(n->type);
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

struct symbol_table *ast_node_symbol_table_get(struct ast_node *n)
{
    switch(n->type) {
    case AST_ROOT:
        return ast_root_symbol_table_get(n);
    case AST_BLOCK:
        return ast_block_symbol_table_get(n);
    case AST_FUNCTION_IMPLEMENTATION:
        return ast_fnimpl_symbol_table_get(n);
    case AST_TYPE_DECLARATION:
        return ast_typedecl_symbol_table_get(n);
    default:
        RF_ASSERT_OR_CRITICAL(false, return NULL,
                              "get_symbol_table() was called on \""RF_STR_PF_FMT"\" which"
                              " is an illegal node for this action",
                              RF_STR_PF_ARG(ast_node_str(n)));
    }
}

/* -- ast_root functions -- */

struct ast_node *ast_root_create(struct inpfile *file)
{
    char *beg;
    struct ast_node *n = NULL;
    
    beg = inpstr_beg(&file->str);
    n = ast_node_create_ptrs(
        AST_ROOT, file, beg,
        beg + inpstr_len_from_beg(&file->str));    
    
    if (!n) {
        RF_ERRNOMEM();
    }

    return n;
}

i_INLINE_INS bool ast_root_symbol_table_init(struct ast_node *n,
                                             struct analyzer *a);
i_INLINE_INS struct symbol_table *ast_root_symbol_table_get(struct ast_node *n);

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
