#ifndef LFR_ANALYZER_PASS1_H
#define LFR_ANALYZER_PASS1_H

#include <stdbool.h>

struct analyzer;
struct ast_node;
struct front_ctx;
struct analyzer_traversal_ctx;

/**
 * This is the first pass of the analyzer phase.
 *
 * Initializes the symbol tables for the nodes where this is needed and also
 * populates them with values. Since this is the first pass of the analyzer
 * phase this function also changes the ownership of the ast nodes.
 *
 * Also creates a hash for all identifiers and string literals and adds
 * them to global string tables.
 * This way they can all be disassociated from the file.
 *
 * @param a           The analyzer handle
 * @param stdlib      If we compile with stdlib this is not NULL and should
 *                    contains the struct front_ctx of the stdlib
 * @return            True for success, false otherwise
 */
bool analyzer_first_pass(struct analyzer *a, struct front_ctx *stdlib);

/**
 * A function to to be called for a node while traversing the AST
 * during analysis. Switches the traversal context's current symbol table
 * to the parent's node symbol table dependin on various condition.
 *
 * @param n           The node the callback is called for
 * @param ctx         The traversal context
 */
bool analyzer_handle_symbol_table_ascending(struct ast_node *n,
                                            struct analyzer_traversal_ctx *ctx);

/**
 * A function to to be called for a node while traversing the AST
 * during analysis. Switches the traversal context's current symbol table to the
 * new symbol table of child @c n if it has one and if all conditions are met.
 * Also creates a new pattern matching context if we are entering a match
 * expression. The destruction of said context is taken care of during the
 * end of typechecking phase of a match expression.
 *
 * @param n           The node the callback is called for
 * @param ctx         The traversal context
 */
bool analyzer_handle_traversal_descending(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx);
#endif
