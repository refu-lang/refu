#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H

#include <stdbool.h>
#include <String/rf_str_decl.h>
#include <Utils/sanity.h>

struct ast_node;
struct inplocation;
struct inplocation_mark;

struct ast_identifier {
    struct RFstring string;
};


struct ast_node *ast_identifier_create(struct inplocation *loc);
struct ast_node *ast_identifier_create(struct inplocation *loc);
void ast_identifier_print(struct ast_node *n, int depth);
struct RFstring *ast_identifier_str(struct ast_node *n);

/**
 * An identifier annotated with extra information
 */
struct ast_xidentifier {
    struct ast_node *id;
    bool is_constant;
    struct ast_node *genr;
};


struct ast_node *ast_xidentifier_create(struct inplocation_mark *start,
                                        struct inplocation_mark *end,
                                        struct ast_node *id,
                                        bool is_constant,
                                        struct ast_node *genr);
#endif
