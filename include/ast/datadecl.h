#ifndef LFR_AST_DATADECL_H
#define LFR_AST_DATADECL_H

#include <RFintrusive_list.h>
#include <parser/tokens.h>

struct ast_node;
struct parser_file;

enum dataop_type {
    DATAOP_INVALID,
    DATAOP_SUM,
    DATAOP_PRODUCT,
    DATAOP_IMPLICATION
};

i_INLINE_DECL const struct RFstring *dataop_type_str(enum dataop_type type)
{
    if (type == DATAOP_SUM) {
        return &parser_tok_dsum;
    } else if (type == DATAOP_PRODUCT) {
        return &parser_tok_dprod;
    } else if (type == DATAOP_IMPLICATION) {
        return &parser_tok_dimpl;
    }

    return NULL;
}

struct ast_dataop {
    enum dataop_type type;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_datadesc {
    bool is_dataop;
    union {
        struct {
            struct ast_node *id;
            struct ast_node *desc;
        };
        struct ast_node *dataop;
    };
};

struct ast_node *ast_datadesc_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *id,
                                     bool dataop);
void ast_datadesc_destroy(struct ast_node *n);
void ast_datadesc_print(struct ast_node *n, int depth, const char *description);

struct ast_node *ast_dataop_create(struct parser_file *f,
                                   char *sp,
                                   char *ep,
                                   enum dataop_type type,
                                   struct ast_node *left,
                                   struct ast_node *right);
void ast_dataop_destroy(struct ast_node *n);
void ast_dataop_print(struct ast_node *n, int depth, const char *description);


void ast_datadesc_set_desc(struct ast_node *n, struct ast_node *d);

struct ast_datadecl {
    //! identifier of the name
    struct ast_node *name;
    //! Data description
    struct ast_node *desc;
};

struct ast_node *ast_datadecl_create(struct parser_file *f,
                                     char *sp,
                                     char *ep,
                                     struct ast_node *name,
                                     struct ast_node *desc);
void ast_datadecl_destroy(struct ast_node *n);


struct RFstring *ast_datadecl_name_str(struct ast_node *n);

void ast_datadecl_print(struct ast_node *n, int depth, const char *description);
#endif
