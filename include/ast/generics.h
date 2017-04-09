#ifndef LFR_AST_GENERICS_H
#define LFR_AST_GENERICS_H

#include <rfbase/defs/inline.h>
#include <ast/generics_decls.h>

struct inplocation_mark;

/* -- genrtype functions -- */

struct ast_node *ast_genrtype_create(struct ast_node *type, struct ast_node *id);

#include <ast/ast.h>

i_INLINE_DECL const struct RFstring *ast_genrtype_type_str(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_GENERIC_TYPE);
    return ast_identifier_str(n->genrtype.type);
}

i_INLINE_DECL const struct RFstring *ast_genrtype_id_str(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_GENERIC_TYPE);
    return ast_identifier_str(n->genrtype.id);
}

/* -- genrdecl functions -- */

struct ast_node *ast_genrdecl_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end);

/**
 * Check if an identifier string is a generic type in the declaration
 *
 * @param n         The generic type declaration
 * @param id        The identifier string to check
 * @return          The ast node of the generic type if found and NULL otherwise
 */
struct ast_node *ast_genrdecl_string_is_genr(struct ast_node *n,
                                             const struct RFstring *id);

/* -- genrattr functions -- */

struct ast_node *ast_genrattr_create(const struct inplocation_mark *start,
                                     const struct inplocation_mark *end);
#endif
