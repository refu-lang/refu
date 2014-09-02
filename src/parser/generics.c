#include <parser/generics.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>
#include <parser/tokens.h>
#include <parser/parser.h>
#include <parser/identifier.h>


static struct ast_node * parser_file_acc_genrtype(struct parser_file *f)
{
    struct ast_node *n;
    struct ast_node *id;
    struct parser_offset proff;
    char *sp;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_sp(f);
    if (!parser_file_acc_string_ascii(f, &parser_kw_type)) {
        parser_file_synerr(f, "Expected a generic type keyword after '<'");
        goto err;
    }

    parser_file_acc_ws(f);
    id = parser_file_acc_identifier(f);
    if (!id) {
        parser_file_synerr(f, "Expected an identifier for the generic type");
        goto err;
    }

    n = ast_genrtype_create(f, sp, parser_file_sp(f), AST_GENR_TYPE, id);
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
    struct ast_node *arg;
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
        ast_genrdecl_add_member(n, genrtype);

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
