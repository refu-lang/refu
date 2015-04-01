#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H

#include <stdbool.h>
#include <String/rf_str_decl.h>

#include <Utils/hash.h>
#include <Utils/sanity.h>

struct ast_node;
struct inplocation;
struct inplocation_mark;
struct analyzer;

struct ast_identifier {
    struct RFstring string;
    uint32_t hash;
};


struct ast_node *ast_identifier_create(struct inplocation *loc);
void ast_identifier_print(struct ast_node *n, int depth);

/**
 * String getter for both an identifier and an xidentifier's string when
 * the identifier is still before the first pass of the analyzer
 */
const struct RFstring *ast_identifier_str(const struct ast_node *n);

/**
 * String getter for both an identifier and an xidentifier's string when
 * the identifier has been indexed by the analyzer
 */
const struct RFstring *ast_identifier_analyzed_str(const struct ast_node *n,
                                                   const struct analyzer *a);
/**
 * Returns if the identifier is '_', which is a wildcard
 */
bool ast_identifier_is_wildcard(const struct ast_node *n);

bool ast_identifier_hash_create(struct ast_node *n, struct analyzer *a);
uint32_t ast_identifier_hash_get_or_create(struct ast_node *n, struct analyzer *a);


/* -- xidentifier -- */

/**
 * An identifier annotated with extra information
 */
struct ast_xidentifier {
    struct ast_node *id;
    bool is_constant;
    struct ast_node *genr;
};


struct ast_node *ast_xidentifier_create(const struct inplocation_mark *start,
                                        const struct inplocation_mark *end,
                                        struct ast_node *id,
                                        bool is_constant,
                                        struct ast_node *genr);

/**
 * String getter for only for an xidentifier's string
 */
const struct RFstring *ast_xidentifier_str(const struct ast_node *n);
#endif
