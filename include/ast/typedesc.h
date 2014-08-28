#ifndef LFR_AST_TYPEDESC_H
#define LFR_AST_TYPEDESC_H

#include <RFintrusive_list.h>
#include <parser/tokens.h>

struct ast_node;
struct parser_file;

enum typeop_type {
    TYPEOP_INVALID,
    TYPEOP_SUM,
    TYPEOP_PRODUCT,
    TYPEOP_IMPLICATION
};

i_INLINE_DECL const struct RFstring *typeop_type_str(enum typeop_type type)
{
    if (type == TYPEOP_SUM) {
        return &parser_tok_dsum;
    } else if (type == TYPEOP_PRODUCT) {
        return &parser_tok_dprod;
    } else if (type == TYPEOP_IMPLICATION) {
        return &parser_tok_dimpl;
    }

    return NULL;
}

struct ast_typeop {
    //! Type Operator type
    enum typeop_type type;
};

struct ast_node *ast_typedesc_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *id);
void ast_typedesc_set_right(struct ast_node *n, struct ast_node *d);
void ast_typedesc_print(struct ast_node *n, int depth, const char *description);

struct ast_node *ast_typeop_create(struct parser_file *f,
                                   char *sp,
                                   char *ep,
                                   enum typeop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right);
void ast_typeop_print(struct ast_node *n, int depth, const char *description);
#endif