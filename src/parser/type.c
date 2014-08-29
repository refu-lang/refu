#include <parser/type.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <parser/tokens.h>

static inline enum typeop_type
parser_file_acc_typeop_token(struct parser_file *f)
{
    parser_file_acc_ws(f);
    if (parser_file_acc_string_ascii(f, &parser_tok_dsum)) {
        return TYPEOP_SUM;
    } else if (parser_file_acc_string_ascii(f, &parser_tok_dprod)) {
        return TYPEOP_PRODUCT;
    } else if (parser_file_acc_string_ascii(f, &parser_tok_dimpl)) {
        return TYPEOP_IMPLICATION;
    }

    return TYPEOP_INVALID;
}

static struct ast_node *parser_file_acc_typedesc_parencolon(
    struct parser_file *f,
    struct ast_node *left,
    struct ast_node *left_identifier,
    enum typeop_type conn_type,
    int *paren_count)
{
    struct ast_node *paren_desc;
    struct ast_node *n = NULL;
    struct parser_offset proff;
    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    if (parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
        *paren_count = *paren_count + 1;
        paren_desc = parser_file_acc_typedesc(f, paren_count);
        if (!paren_desc) {
            parser_file_synerr(
                f, "Expected a data description right of \"(\"");
            goto not_found;
        }
        n = ast_typedesc_create(f, ast_node_startsp(left_identifier),
                                parser_file_sp(f), left_identifier);
        if (!n) { /* error */
            ast_node_destroy(paren_desc);
            goto not_found;
        }
        ast_typedesc_set_right(&n->typedesc, paren_desc);
        if (left) {
            return ast_typeop_create(f, ast_node_startsp(left),
                                     parser_file_sp(f),
                                     conn_type, left, n);
        }
        //else just return n
    }

    return n;

not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

static struct ast_node *parser_file_acc_typedesc_single(struct parser_file *f,
                                                        struct ast_node *left,
                                                        enum typeop_type conn_type,
                                                        int *paren_count)
{
    enum typeop_type dtype;
    struct ast_node *n;
    struct ast_node *tmp;
    struct ast_node *typeop;
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
        /* parentheses right after ':' */
        if ((n = parser_file_acc_typedesc_parencolon(f,
                                                     left,
                                                     tmp,
                                                     conn_type,
                                                     paren_count))) {
            if (left) {
                return ast_typeop_create(f, ast_node_startsp(left),
                                         parser_file_sp(f),
                                         conn_type, left, n);
            }
            //else
            return n;
        }
        n = ast_typedesc_create(f, sp, NULL, tmp);
        if (!n) { /* error */
            goto not_found;
        }
        tmp = parser_file_acc_typedesc_single(f, NULL,
                                              TYPEOP_INVALID, paren_count);

        if (!tmp) {
            parser_file_synerr(
                f, "Expected a data description right of \":\"");
            goto err_free_this; //TODO: fucked up order of freeing
        }
        ast_typedesc_set_right(&n->typedesc, tmp);
        ast_node_set_end(n, parser_file_sp(f));
        if (left) {
            return ast_typeop_create(f, ast_node_startsp(left),
                                     parser_file_sp(f),
                                     conn_type, left, n);
        }
    } else {
        // depending on context we can have a type description being only
        // an identifier
        n = ast_typedesc_create(f, sp, parser_file_sp(f), tmp);
        if (!n) {
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

struct ast_node *parser_file_acc_typedesc(struct parser_file *f,
                                          int *paren_count)
{
    enum typeop_type dtype = TYPEOP_INVALID;
    struct ast_node *n = NULL;
    struct ast_node *last = NULL;
    struct parser_offset proff;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);
    do {
        n = parser_file_acc_typedesc_single(f, last, dtype, paren_count);
        last = n;
        parser_file_acc_ws(f);
        if (parser_file_acc_string_ascii(f, &parser_tok_cparen)) {
            *paren_count = *paren_count - 1;
            return n;
        }
    } while(((dtype = parser_file_acc_typeop_token(f)) != TYPEOP_INVALID));

    return n;
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_typedecl(struct parser_file *f)
{
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *desc;
    struct parser_offset proff;
    char *sp;
    char *ep;
    int paren_count = 0;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_kw_type)) {
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

    desc = parser_file_acc_typedesc(f, &paren_count);
    if (!desc) {
        parser_file_synerr(f, "Expected data description for data declaration "
                           "of \""RF_STR_PF_FMT"\"",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        ast_node_destroy(name);
        goto not_found;
    }
    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_typedecl_create(f, sp, NULL, name, desc);
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
    ast_typedecl_destroy(data_decl);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}
