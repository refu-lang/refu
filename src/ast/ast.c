#include <ast/ast.h>

#include <rfbase/utils/sanity.h>
#include <rfbase/utils/build_assert.h>
#include <rfbase/utils/memory.h>
#include <rfbase/string/core.h>

#include <ast/block.h>
#include <ast/function.h>
#include <ast/type.h>
#include <ast/matchexpr.h>
#include <ast/forexpr.h>
#include <ast/module.h>
#include <ast/typeclass.h>

static const struct RFstring ast_type_strings[] = {
    [AST_ROOT] = RF_STRING_STATIC_INIT("root"),
    [AST_ARRAY_SPEC] = RF_STRING_STATIC_INIT("array specifier"),
    [AST_BLOCK] = RF_STRING_STATIC_INIT("block"),
    [AST_VARIABLE_DECLARATION] = RF_STRING_STATIC_INIT("variable declaration"),
    [AST_RETURN_STATEMENT] = RF_STRING_STATIC_INIT("return statement"),
    [AST_TYPE_DECLARATION] = RF_STRING_STATIC_INIT("type declaration"),
    [AST_TYPE_OPERATOR] = RF_STRING_STATIC_INIT("type operator"),
    [AST_TYPE_LEAF] = RF_STRING_STATIC_INIT("type leaf"),
    [AST_TYPE_DESCRIPTION] = RF_STRING_STATIC_INIT("type description"),
    [AST_TYPECLASS_DECLARATION] = RF_STRING_STATIC_INIT("typeclass declaration"),
    [AST_TYPECLASS_INSTANCE] = RF_STRING_STATIC_INIT("typeclass instance"),
    [AST_GENERIC_DECLARATION] = RF_STRING_STATIC_INIT("generic declaration"),
    [AST_GENERIC_TYPE] = RF_STRING_STATIC_INIT("generic type"),
    [AST_GENERIC_ATTRIBUTE] = RF_STRING_STATIC_INIT("generic attribute"),
    [AST_FUNCTION_DECLARATION] = RF_STRING_STATIC_INIT("function declaration"),
    [AST_FUNCTION_IMPLEMENTATION] = RF_STRING_STATIC_INIT("function implementation"),
    [AST_FUNCTION_CALL] = RF_STRING_STATIC_INIT("function call"),
    [AST_INDEX_ACCESS] = RF_STRING_STATIC_INIT("index access"),
    [AST_CONDITIONAL_BRANCH] = RF_STRING_STATIC_INIT("conditional branch"),
    [AST_IF_EXPRESSION] = RF_STRING_STATIC_INIT("if expression"),
    [AST_FOR_EXPRESSION] = RF_STRING_STATIC_INIT("for expression"),
    [AST_ITERABLE] = RF_STRING_STATIC_INIT("iterable"),
    [AST_MATCH_EXPRESSION] = RF_STRING_STATIC_INIT("match expression"),
    [AST_MATCH_CASE] = RF_STRING_STATIC_INIT("match case"),
    [AST_MODULE] = RF_STRING_STATIC_INIT("module"),
    [AST_IMPORT] = RF_STRING_STATIC_INIT("import statement"),
    [AST_XIDENTIFIER] = RF_STRING_STATIC_INIT("annotated identifier"),
    [AST_BINARY_OPERATOR] = RF_STRING_STATIC_INIT("binary operator"),
    [AST_UNARY_OPERATOR] = RF_STRING_STATIC_INIT("unary operator"),
    [AST_BRACKET_LIST] = RF_STRING_STATIC_INIT("bracket list"),
    [AST_STRING_LITERAL] = RF_STRING_STATIC_INIT("string literal"),
    [AST_IDENTIFIER] = RF_STRING_STATIC_INIT("identifier"),
    [AST_CONSTANT] = RF_STRING_STATIC_INIT("constant number"),
    [AST_PLACEHOLDER] = RF_STRING_STATIC_INIT("placeholder"),
};

static const struct RFstring ast_state_strings[] = {
    [AST_NODE_STATE_CREATED] = RF_STRING_STATIC_INIT("CREATED"),
    [AST_NODE_STATE_AFTER_PARSING] = RF_STRING_STATIC_INIT("AFTER_PARSING"),
    [AST_NODE_STATE_ANALYZER_PASS1] = RF_STRING_STATIC_INIT("ANALYZER_PASS1"),
    [AST_NODE_STATE_RIR_END] = RF_STRING_STATIC_INIT("RIR_END")
};

#define AST_NODE_IS_LEAF(node_) ((node_)->type >= AST_STRING_LITERAL)

void ast_node_init(struct ast_node *n, enum ast_type type)
{
    RF_STRUCT_ZERO(n);
    n->state = AST_NODE_STATE_CREATED;
    n->type = type;
    darray_init(n->children);
}

struct ast_node *ast_node_create(enum ast_type type)
{
   struct ast_node *ret;
   RF_MALLOC(ret, sizeof(struct ast_node), return NULL);
   ast_node_init(ret, type);
   return ret;
}

struct ast_node *ast_node_create_loc(
    enum ast_type type,
    const struct inplocation *loc)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    inplocation_copy(&ret->location, loc);

    return ret;
}

struct ast_node *ast_node_create_marks(
    enum ast_type type,
    const struct inplocation_mark *start,
    const struct inplocation_mark *end)
{

    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), return NULL);

    ast_node_init(ret, type);
    inplocation_init_marks(&ret->location, start, end);

    return ret;
}

struct ast_node *ast_node_create_ptrs(
    enum ast_type type,
    struct inpfile *f,
    char *sp,
    char *ep)
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
    // type specific destruction  -- only if owned by analyzer and after
    if (n->state >= AST_NODE_STATE_ANALYZER_PASS1) {
        switch(n->type) {
        case AST_ROOT:
            symbol_table_deinit(ast_root_symbol_table_get(n));
            break;
        case AST_BLOCK:
            symbol_table_deinit(ast_block_symbol_table_get(n));
            break;
        case AST_FUNCTION_DECLARATION:
            ast_fndecl_deinit(n);
            break;
        case AST_TYPE_DESCRIPTION:
            symbol_table_deinit(ast_typedesc_symbol_table_get(n));
            break;
        case AST_TYPECLASS_DECLARATION:
            symbol_table_deinit(ast_typeclass_symbol_table_get(n));
            break;
        case AST_TYPECLASS_INSTANCE:
            symbol_table_deinit(ast_typeinstance_symbol_table_get(n));
            break;
        case AST_MODULE:
            symbol_table_deinit(ast_module_symbol_table_get(n));
            break;
        case AST_FOR_EXPRESSION:
            symbol_table_deinit(ast_forexpr_symbol_table_get(n));
            break;
        default:
            // no type specific destruction for the rest
            break;
        }
    }

    // destroy data that are created during typechecking
    if (n->state >= AST_NODE_STATE_TYPECHECK_1) {
        if (n->type == AST_BRACKET_LIST) {
            darray_free(n->bracketlist.members);
        } else if (n->type == AST_FUNCTION_CALL) {
            darray_free(n->fncall.arguments);
        }
    }

    struct ast_node **child;
    darray_foreach(child, n->children) {
        ast_node_destroy(*child);
    }
    darray_free(n->children);

    // free node unless it's a value node still at lexing/parsing phase, or
    // unless it's the placeholder node
    if (!((n->state == AST_NODE_STATE_CREATED && ast_node_has_value(n)) ||
          n->type == AST_PLACEHOLDER)) {
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

void ast_node_add_child(struct ast_node *parent, struct ast_node *child)
{
    darray_append(parent->children, child);
}

void ast_node_copy_children(struct ast_node *parent, struct arr_ast_nodes *children)
{
    RF_ASSERT(
        darray_empty(parent->children) && parent->children.alloc == 0,
        "Make sure that this is only called for empty, unitnialized arrays"
    );
    darray_shallow_copy(parent->children, *children);
}

const struct RFstring *ast_node_get_name_str(const struct ast_node *n)
{
    switch(n->type) {
    case AST_IDENTIFIER:
        return ast_identifier_str(n);
    case AST_XIDENTIFIER:
        return ast_xidentifier_str(n);
    default:
        RF_ASSERT_OR_CRITICAL(
            false, return NULL,
            "Requesting identifier string from illegal"
            "ast node type \""RFS_PF"\"",
            RFS_PA(ast_node_str(n))
        );
    }
}

i_INLINE_INS struct ast_node *ast_node_get_child(struct ast_node *n,
                                                  unsigned int num);
i_INLINE_INS unsigned int ast_node_get_children_number(const struct ast_node *n);
i_INLINE_INS const char *ast_node_startsp(const struct ast_node *n);
i_INLINE_INS const char *ast_node_endsp(const struct ast_node *n);
i_INLINE_INS const struct inplocation *ast_node_location(const struct ast_node *n);
i_INLINE_INS const struct inplocation_mark *ast_node_startmark(const struct ast_node *n);
i_INLINE_INS const struct inplocation_mark *ast_node_endmark(const struct ast_node *n);
i_INLINE_INS enum ast_type ast_node_type(const struct ast_node *n);
i_INLINE_INS bool ast_node_has_value(const struct ast_node *n);
i_INLINE_INS const struct type *ast_node_get_type(const struct ast_node *n);
i_INLINE_INS const struct type *ast_node_get_type_or_die(const struct ast_node *n);
i_INLINE_INS const struct type *ast_node_get_type_or_nil(const struct ast_node *n);

const struct RFstring *ast_nodetype_str(enum ast_type type)
{
    return &ast_type_strings[type];
}

const struct RFstring *ast_nodestate_str(enum ast_node_state state)
{
    return &ast_state_strings[state];
}

const struct RFstring *ast_node_str(const struct ast_node *n)
{
    // assert that the array size is same as enum size
    BUILD_ASSERT(
        sizeof(ast_type_strings)/sizeof(struct RFstring) == AST_TYPES_COUNT
    );
    return ast_nodetype_str(n->type);
}

static struct ast_node g_ast_node_placeholder = {
    .type=AST_PLACEHOLDER,
    .state=AST_NODE_STATE_CREATED,
    .expression_type=NULL,
    .location={.start=LOCMARK_INIT_ZERO(), .end=LOCMARK_INIT_ZERO()},
    .children={{NULL, 0, 0}}
};
struct ast_node *ast_node_placeholder()
{
    return &g_ast_node_placeholder;
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
            printf("|----> "RFS_PF" "INPLOCATION_FMT2"\n",
                   RFS_PA(ast_node_str(n)),
                   INPLOCATION_ARG2(f, &n->location));
        } else {
            printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
            printf("|----> "RFS_PF" "INPLOCATION_FMT2"\n",
                   RFS_PA(ast_node_str(n)),
                   INPLOCATION_ARG2(f, &n->location));
        }
    } else {
        printf("%*.*s "INPLOCATION_FMT2"\n",
               depth * AST_PRINT_DEPTHMUL,
               RFS_PA(ast_node_str(n)),
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
    case AST_TYPE_DESCRIPTION:
        return ast_typedesc_symbol_table_get(n);
    case AST_MATCH_CASE:
        return ast_matchcase_symbol_table_get(n);
    case AST_MODULE:
        return ast_module_symbol_table_get(n);
    case AST_FOR_EXPRESSION:
        return ast_forexpr_symbol_table_get(n);
    case AST_TYPECLASS_DECLARATION:
        return ast_typeclass_symbol_table_get(n);
    case AST_TYPECLASS_INSTANCE:
        return ast_typeinstance_symbol_table_get(n);
    default:
        RF_ASSERT_OR_CRITICAL(
            false, return NULL,
            "get_symbol_table() was called on \""RFS_PF"\" which"
            " is an illegal node for this action",
            RFS_PA(ast_node_str(n))
        );
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

i_INLINE_INS struct symbol_table *ast_root_symbol_table_get(struct ast_node *n);

void ast_print(struct ast_node *n, struct inpfile *f, int depth)
{
    struct ast_node **c;
    ast_print_prelude(n, f, depth, "");

    switch(n->type) {
    case AST_IDENTIFIER:
        ast_identifier_print(n, depth + 1);
        break;
    default:
        printf(RFS_PF"\n", RFS_PA(ast_node_str(n)));
        darray_foreach(c, n->children) {
            ast_print(*c, f, depth + 1);
        }
        break;
    }
}
