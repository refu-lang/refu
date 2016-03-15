#ifndef LFR_ANALYZER_TYPECHECK_H
#define LFR_ANALYZER_TYPECHECK_H

#include <stdbool.h>

#include <rflib/utils/sanity.h>
#include <rflib/defs/inline.h>

struct module;
struct ast_node;
struct type;
struct analyzer_traversal_ctx;

bool analyzer_typecheck(struct module *m, struct ast_node *n);
/**
 * Convenience function to set the type of a node and
 * remember last node type during traversal
 */
void traversal_node_set_type(struct ast_node *n,
                             const struct type *t,
                             struct analyzer_traversal_ctx *ctx);
#endif
