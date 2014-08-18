#include <parser/generics.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>
#include <parser/tokens.h>
#include <parser/parser.h>

struct ast_node *parser_file_acc_genrdecl(struct parser_file *f)
{
    struct ast_node *n;
    struct ast_node *id;
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
        parser_file_acc_ws(f);
        if (!parser_file_acc_string_ascii(f, &parser_kw_type)) {
            parser_file_synerr(f, "Expected a generic type keyword after '<'");
            goto err_free;
        }

        parser_file_acc_ws(f);
        id = parser_file_acc_identifier(f);
        if (!id) {
            parser_file_synerr(f, "Expected an identifier for the generic type");
            goto err_free;
        }
        ast_genrdecl_add_member(n, AST_GENR_TYPE, id);

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
    return NULL;
}
