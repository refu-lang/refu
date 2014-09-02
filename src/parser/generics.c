#include <parser/generics.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>
#include <parser/tokens.h>
#include <parser/parser.h>
#include <parser/identifier.h>
#include <parser/type.h>


//failure in this means syntax error
static struct ast_node * parser_file_acc_genrtype(struct parser_file *f)
{
    struct ast_node *type_id;
    struct ast_node *id_id;
    struct ast_node *n = NULL;
    struct parser_offset proff;
    char *sp;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);
    type_id = parser_file_acc_identifier(f);
    if (!type_id) {
        parser_file_synerr(f,
                           "Expected an identifier for the generic type kind");
        goto err;
    }

    parser_file_acc_ws(f);
    id_id = parser_file_acc_identifier(f);
    if (!id_id) {
        parser_file_synerr(f,
                           "Expected an identifier for the generic type name");
        goto err;
    }

    n = ast_genrtype_create(f, sp, parser_file_sp(f), type_id, id_id);
    /* if !n, then NULL will be returned anyway */

    return n;
err:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_genrdecl(struct parser_file *f)
{
    struct ast_node *n;
    struct ast_node *genrtype;
    struct parser_offset proff;
    bool found_comma;
    char *sp;
    char *ep;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_tok_lt)) {
        goto not_found;
    }

    n = ast_genrdecl_create(f, sp, NULL);

    do {
        found_comma = false;
        genrtype = parser_file_acc_genrtype(f);
        if (!genrtype) {
            goto err_free;
        }
        ast_node_add_child(n, genrtype);

        if (parser_file_acc_string_ascii(f, &parser_tok_comma)) {
            found_comma = true;
        }

    } while(found_comma);

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_gt)) {
        parser_file_synerr(f,
                           "Expected a closing '>' at generic declaration");
        goto err_free;
    }

    ast_node_set_end(n, parser_file_sp(f));
    return n;

err_free:
    ast_node_destroy(n);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_genrattr(struct parser_file *f)
{
    struct ast_node *n;
    struct ast_node *child;
    struct parser_offset proff;
    bool found_comma;
    char *sp;
    char *ep;
    int paren_count;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);

    if (!parser_file_acc_string_ascii(f, &parser_tok_lt)) {
        goto not_found;
    }

    n = ast_genrattr_create(f, sp, NULL);

    do {
        found_comma = false;
        if (parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
            paren_count = 1;
            child = parser_file_acc_typedesc(f, &paren_count);
            if (!child) {
                parser_file_synerr(
                    f,
                    "Expected a type description after '('"
                );
                goto err_free;
            }
            if (paren_count != 0 &&
                !parser_file_acc_string_ascii(f, &parser_tok_cparen)) {

                parser_file_synerr(
                    f,
                    "Expected a ')' after type description"
                );
                ast_node_destroy(child);
                goto err_free;
            }
        } else if(!(child = parser_file_acc_xidentifier(f))) {
                parser_file_synerr(
                    f,
                    "Expected an either an annotated identifier or a "
                    "parenthesized type description"
                );
                goto err_free;
        }

        ast_node_add_child(n, child);
        if (parser_file_acc_string_ascii(f, &parser_tok_comma)) {
            found_comma = true;
        }
    } while(found_comma);

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_gt)) {
        parser_file_synerr(f,
                           "Expected a closing '>' at generic declaration");
        goto err_free;
    }

    ast_node_set_end(n, parser_file_sp(f));
    return n;

err_free:
    ast_node_destroy(n);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}
