#include <parser/data.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <parser/tokens.h>

static inline enum dataop_type
parser_file_acc_dataop_token(struct parser_file *f)
{
    if (parser_file_acc_string_ascii(f, &parser_tok_dsum)) {
        return DATAOP_SUM;
    } else if (parser_file_acc_string_ascii(f, &parser_tok_dprod)) {
        return DATAOP_PRODUCT;
    } else if (parser_file_acc_string_ascii(f, &parser_tok_dimpl)) {
        return DATAOP_IMPLICATION;
    }

    return DATAOP_INVALID;
}

static struct ast_node *parser_file_acc_dataop(struct parser_file *f,
                                        struct ast_node *left,
                                        enum dataop_type type)
{
    struct ast_node *n;
    struct ast_node *right;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);


    n = ast_datadesc_create(f, ast_node_startsp(left), NULL, left, true);
    if (!n) { //TODO: error
        goto not_found;
    }

    right = parser_file_acc_datadesc(f, NULL);


not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_datadesc(struct parser_file *f,
                                          struct ast_node *left)
{
    enum dataop_type dtype;
    struct ast_node *n;
    struct ast_node *tmp;
    struct ast_node *dataop;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    tmp = parser_file_acc_identifier(f);
    if (!tmp) {
        goto not_found;
    }

    parser_file_acc_ws(f);
    if (parser_file_acc_string_ascii(f, &parser_tok_colon)) {
        n = ast_datadesc_create(f, sp, NULL, tmp, false);
        if (!n) { /* error */
            goto not_found;
        }
        tmp = parser_file_acc_datadesc(f, NULL);

        if (!tmp) {
            parser_file_synerr(
                f, "Expected a data description right of \":\"");
            goto err_free_this; //TODO: fucked up order of freeing
        }
        ast_datadesc_set_desc(n, tmp);
        ast_node_set_end(n, parser_file_sp(f));
    } else if ((dtype = parser_file_acc_dataop_token(f)) != DATAOP_INVALID) {
        if (!left) {
            parser_file_synerr(
                f, "Expected a data description left of "RF_STR_PF_FMT,
                RF_STR_PF_ARG(dataop_type_str(dtype)));
            goto not_found;
        }

        dataop = parser_file_acc_dataop(f, left, dtype);
        if (!dataop) {
            //TODO: what error here?
            goto not_found;
        }
        n = ast_datadesc_create(f, sp, parser_file_sp(f), dataop, true);
        if (!n) {
            ast_node_destroy(dataop);
            goto not_found;
        }
    } else {
        if (!left) {
            // depending on context we can have a type description being only
            // an identifier
            n = ast_datadesc_create(f, sp, parser_file_sp(f), tmp, false);
            if (!n) {
                ast_node_destroy(tmp);
                goto not_found;
            }
        } else {
            parser_file_synerr(f, "Expected either a data operator or \":\""
                               "on the left of \""RF_STR_PF_FMT"\"",
                               RF_STR_PF_ARG(ast_identifier_str(tmp)));
            ast_node_destroy(tmp);
            goto not_found;
        }
    }


    return n;

err_free_this:
    ast_node_destroy(n);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_datadecl(struct parser_file *f)
{
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *desc;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_kw_data)) {
        goto not_found;
    }

    name = parser_file_acc_identifier(f);
    if (!name) {
        goto not_found;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_ocbrace)) {
        goto not_found;
    }

    desc = parser_file_acc_datadesc(f, NULL);
    if (!desc) {
        parser_file_synerr(f, "Expected data description for data declaration "
                           "of \""RF_STR_PF_FMT"\"", ast_identifier_str(name));
        ast_node_destroy(name);
        goto not_found;
    }
    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_datadecl_create(f, sp, NULL, name, desc);
    if (!data_decl) {//memory error
        ast_node_destroy(name);
        ast_node_destroy(desc);
        goto not_found;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_ccbrace)) {
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing brace '}' in data "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }
    ast_node_set_end(data_decl, parser_file_sp(f));

    return data_decl;

err_free:
    ast_datadecl_destroy(data_decl);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}
