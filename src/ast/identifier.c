#include <ast/identifier.h>

#include <rfbase/utils/sanity.h>
#include <rfbase/string/core.h>

#include <ast/ast.h>
#include <module.h>
#include <types/type.h>
#include <utils/common_strings.h>

struct ast_node *ast_identifier_create(struct inplocation *loc, unsigned skip_start)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_IDENTIFIER, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(
        &ret->identifier.string,
        loc->start.p + skip_start,
        loc->end.p - loc->start.p + 1 - skip_start
    );

    return ret;
}

void ast_identifier_print(struct ast_node *n, int depth)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("Value: \""RFS_PF"\"\n", RFS_PA(&n->identifier.string));
}

const struct RFstring *ast_identifier_str(const struct ast_node *n)
{
    RF_ASSERT(
        n->type == AST_IDENTIFIER || n->type == AST_XIDENTIFIER,
        "Unexpected ast node type"
    );
    if (n->type == AST_IDENTIFIER) {
        return &n->identifier.string;
    }
    return ast_xidentifier_str(n);
}

const struct RFstring *ast_identifier_analyzed_str(const struct ast_node *n)
{
    RF_ASSERT(n->type == AST_IDENTIFIER || n->type == AST_XIDENTIFIER,
              "Unexpected ast node type");
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_ANALYZER_PASS1);

    RF_CRITICAL_FAIL("function not implemented yet");
    return NULL;
#if 0 // this is not used anywhere atm. If that changes either pass the module
      // or each node should know which module it belongs to.
    if (n->type == AST_IDENTIFIER) {
        return string_table_get_str(a->identifiers_table, n->identifier.hash);
    }
    return string_table_get_str(a->identifiers_table,
                                n->xidentifier.id->identifier.hash);
#endif
}

bool ast_identifier_hash_create(struct ast_node *n, struct module *m)
{
    return rf_objset_add(&m->identifiers_set, string, &n->identifier.string);
}

bool string_is_wildcard(const struct RFstring *s)
{
    return rf_string_equal(s, &g_str_wildcard);
}

bool ast_identifier_is_wildcard(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    return string_is_wildcard(&n->identifier.string);
}

bool ast_identifier_is_self(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    return rf_string_equal(&n->identifier.string, &g_str_self);
}

bool ast_identifier_is_Type(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    return rf_string_equal(&n->identifier.string, &g_str_Type);
}

/* -- xidentifier functions -- */

struct ast_node *ast_xidentifier_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *id,
    bool is_constant,
    struct ast_node *genr,
    struct ast_node *arrspec
)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_XIDENTIFIER, start, end);
    if (!ret) {
        return NULL;
    }

    ast_node_add_child(ret, id);
    ret->xidentifier.is_constant = is_constant;
    ret->xidentifier.id = id;
    ret->xidentifier.genr = genr;
    ret->xidentifier.arrspec = arrspec;
    if (genr) {
        ast_node_add_child(ret, genr);
    }
    if (arrspec) {
        ast_node_add_child(ret, arrspec);
    }

    return ret;
}

const struct RFstring *ast_xidentifier_str(const struct ast_node *n)
{
    return &n->xidentifier.id->identifier.string;
}
