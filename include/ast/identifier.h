#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H

#include <stdbool.h>

#include <rfbase/string/decl.h>
#include <rfbase/utils/hash.h>
#include <rfbase/utils/sanity.h>

struct ast_node;
struct inplocation;
struct inplocation_mark;
struct module;

struct ast_identifier {
    struct RFstring string;
    uint32_t hash;
};


/**
 * Create a new AST identifier
 *
 * @param loc            The location from which to create the identifier
 * @param skip_start     The number of chars to skip from the location start
 *                       after which the actual identifier content starts.
 *                       Can be 0.
 * @return               The allocated identifier.
 */
struct ast_node *ast_identifier_create(struct inplocation *loc, unsigned skip_start);
void ast_identifier_print(struct ast_node *n, int depth);

/**
 * String getter for both an identifier and an xidentifier's string when
 * the identifier is still before the first pass of the analysis stage
 */
const struct RFstring *ast_identifier_str(const struct ast_node *n);

/**
 * String getter for both an identifier and an xidentifier's string when
 * the identifier has been indexed by the analysis stage
 */
const struct RFstring *ast_identifier_analyzed_str(const struct ast_node *n);

/**
 * Returns if the string is '_', which is a wildcard
 */
bool string_is_wildcard(const struct RFstring *s);

/**
 * Returns if the identifier is '_', which is a wildcard
 */
bool ast_identifier_is_wildcard(const struct ast_node *n);

/**
 * Returns if this is the special 'self' identifier
 */
bool ast_identifier_is_self(const struct ast_node *n);

bool ast_identifier_hash_create(struct ast_node *n, struct module *m);

/* -- xidentifier -- */

/**
 * An identifier annotated with extra information
 */
struct ast_xidentifier {
    struct ast_node *id;
    bool is_constant;
    struct ast_node *genr;
    struct ast_node *arrspec;
};


struct ast_node *ast_xidentifier_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    struct ast_node *id,
    bool is_constant,
    struct ast_node *genr,
    struct ast_node *arrspec
);

/**
 * String getter for only for an xidentifier's string
 */
const struct RFstring *ast_xidentifier_str(const struct ast_node *n);
#endif
