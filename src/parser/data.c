#include <parser/data.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <parser/tokens.h>


struct ast_node *parser_file_acc_datadecl(struct parser_file *f)
{
    struct ast_node *data_decl;
    struct ast_node *name;
    struct ast_node *member;
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

    /* from here and on we throw syntax errors if something goes wrong */
    data_decl = ast_datadecl_create(f, sp, NULL, name);

    while ((member = parser_file_acc_vardecl(f)) != NULL) {
        ast_datadecl_add_member(data_decl, member);
    }
    if (parser_file_has_synerr(f)) {
        parser_file_synerr(f,
                           "Expected either a variable declaration or "
                           "a closing brace '}' in data "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
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
