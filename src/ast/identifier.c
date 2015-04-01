#include <ast/identifier.h>

#include <ast/ast.h>
#include <analyzer/analyzer.h>
#include <analyzer/string_table.h>

#include <Utils/sanity.h>


struct ast_node *ast_identifier_create(struct inplocation *loc)
{
    struct ast_node *ret;
    ret = ast_node_create_loc(AST_IDENTIFIER, loc);
    if (!ret) {
        return NULL;
    }
    RF_STRING_SHALLOW_INIT(&ret->identifier.string, loc->start.p,
                           loc->end.p - loc->start.p + 1);

    return ret;
}

void ast_identifier_print(struct ast_node *n, int depth)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    printf("%*s", depth * AST_PRINT_DEPTHMUL, " ");
    printf("Value: \""RF_STR_PF_FMT"\"\n",
               RF_STR_PF_ARG(&n->identifier.string));
}

const struct RFstring *ast_identifier_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER || AST_XIDENTIFIER);
    if (n->type == AST_IDENTIFIER) {
        return &n->identifier.string;
    }
    return ast_xidentifier_str(n);
}

const struct RFstring *ast_identifier_analyzed_str(const struct ast_node *n,
                                                   const struct analyzer *a)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER || AST_XIDENTIFIER);
    RF_ASSERT(n->state >= AST_NODE_STATE_ANALYZER_PASS1,
              "calling function at wrong part of processing pipeline");

    if (n->type == AST_IDENTIFIER) {
        return string_table_get_str(a->identifiers_table, n->identifier.hash);
    }
    return string_table_get_str(a->identifiers_table,
                                n->xidentifier.id->identifier.hash);
}

bool ast_identifier_hash_create(struct ast_node *n, struct analyzer *a)
{
    return string_table_add_or_get_str(a->identifiers_table,
                                       &n->identifier.string,
                                       &n->identifier.hash);
}

uint32_t ast_identifier_hash_get_or_create(struct ast_node *n, struct analyzer *a)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    if (n->state == AST_NODE_STATE_AFTER_PARSING) {
        ast_identifier_hash_create(n, a);
    }
    return n->identifier.hash;
}

bool ast_identifier_is_wildcard(const struct ast_node *n)
{
    static const struct RFstring wildcard_s = RF_STRING_STATIC_INIT("_");
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    return rf_string_equal(&n->identifier.string, &wildcard_s);
}

/* -- xidentifier functions -- */

struct ast_node *ast_xidentifier_create(const struct inplocation_mark *start,
                                        const struct inplocation_mark *end,
                                        struct ast_node *id,
                                        bool is_constant,
                                        struct ast_node *genr)
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
    if (genr) {
        ast_node_add_child(ret, genr);
    }

    return ret;
}

const struct RFstring *ast_xidentifier_str(const struct ast_node *n)
{
    return &n->xidentifier.id->identifier.string;
}
