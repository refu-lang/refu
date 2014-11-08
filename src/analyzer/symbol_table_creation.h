#ifndef LFR_ANALYZER_SYMBOL_TABLE_CREATION_H
#define LFR_ANALYZER_SYMBOL_TABLE_CREATION_H

#include <stdbool.h>

struct analyzer;
struct ast_node;
struct analyzer_traversal_ctx;

/**
 * Initializes the symbol tables for the nodes where this is needed and also
 * populates them with values. Since this is the first pass of the analyzer
 * phase this function also changes the ownership of the ast nodes.
 *
 * @param a     The analyzer handle
 * @return      True for success, false otherwise
 */
bool analyzer_create_symbol_tables(struct analyzer *a);

/**
 * A callback function to to be called for a node while traversing the AST
 * during analysis. Switches the traversal context's current symbol table
 * to the parent's node symbol table
 *
 * @param n           The node the callback is called for
 * @param ctx         The traversal context
 */
bool analyzer_make_parent_st_current(struct ast_node *n,
                                     struct analyzer_traversal_ctx *ctx);

#endif
