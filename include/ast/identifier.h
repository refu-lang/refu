#ifndef LFR_AST_IDENTIFIER_H
#define LFR_AST_IDENTIFIER_H

#include <stdbool.h>
#include <String/rf_str_decl.h>
#include <Utils/sanity.h>

struct parser_file;
struct ast_node;

struct ast_identifier {
    struct RFstring string;
};


struct ast_node *ast_identifier_create(struct parser_file *file,
                                       char *sp, char *ep);
void ast_identifier_print(struct ast_node *n, int depth);
struct RFstring *ast_identifier_str(struct ast_node *n);

/**
 * An identifier annotated with extra information
 */
struct ast_xidentifier {
    struct ast_identifier *id;
    // TODO: determine what and how these annotations should be
    bool constant;
    bool pointer;
};


struct ast_node *ast_xidentifier_create(struct parser_file *f,
                                        char *sp, char *ep,
                                        struct ast_node *id,
                                        bool constant, bool pointer);
#endif
