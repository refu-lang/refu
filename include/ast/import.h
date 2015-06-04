#ifndef LFR_AST_IMPORT_H
#define LFR_AST_IMPORT_H

#include <ast/ast.h>

#include <Definitions/inline.h>
#include <Utils/sanity.h>
#include <lexer/tokens.h>

struct ast_node;
struct inplocation_mark;

struct ast_node *ast_import_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   bool foreign);

i_INLINE_DECL bool ast_import_is_foreign(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_IMPORT);
    return n->import.foreign;
}

/**
 * Specialized (bit of a hack) function to return special foreign fncall function
 */
const struct ast_node *ast_foreign_fncall();
#endif
