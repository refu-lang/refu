#ifndef LFR_TYPES_TYPE_UTILS_H
#define LFR_TYPES_TYPE_UTILS_H

#include <rflib/defs/inline.h>

#include <types/type_elementary.h>
#include <ast/ast.h>

i_INLINE_DECL bool ast_node_is_elementary_identifier(struct ast_node *n)
{
    return n->type == AST_IDENTIFIER && type_is_simple_elementary(n->expression_type);
}

#endif
