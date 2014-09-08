#include "function.h"

#include <stdlib.h>

struct ast_node *parser_acc_fndecl(struct parser *p)
{
    return NULL;
//TODO
#if 0
    struct ast_node *fn;
    struct ast_node *name;
    struct ast_node *genr;
    struct parser_offset proff;
    char *sp;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = parser_file_p(f);

    if (!parser_file_acc_string_ascii(f, &parser_tok_fn)) {
        goto not_found;
    }

    /* error from here and on */
    parser_file_acc_ws(f);
    name = parser_file_acc_identifier(f);
    if (!name) {
        goto not_found;
    }
    fn = ast_fndecl_create(f, sp, NULL, name);

    /* optional: Generic declaration */
    parser_file_acc_ws(f);
    genr = parser_file_acc_genrdecl(f);
    if (genr) {
        ast_fndecl_set_genr(fn, genr);
    } else if (!genr && parser_file_has_synerr(f)) { /* error */
        goto err_free;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
        goto err_free;
    }

    if (!parser_file_acc_commsep_args(f, &fn->fndecl.args)) {
        goto err_free;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_cparen)) {
        parser_file_synerr(f, 0,
                           "Expected a closing parentheses ')' at function "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(name)));
        goto err_free;
    }

    parser_file_acc_ws(f);
    if (!parser_file_acc_string_ascii(f, &parser_tok_arrow)) {
        /* no return value */
        ast_node_set_end(fn, parser_file_p(f));
        return fn;
    }

    parser_file_acc_ws(f);
    name = parser_file_acc_identifier(f);
    if (!name) {
        parser_file_synerr(f, 0,
                           "Expected a return type for function  "
                           "declaration for '"RF_STR_PF_FMT"'",
                           RF_STR_PF_ARG(ast_identifier_str(fn->fndecl.name)));
        goto err_free;
    }

    ast_fndecl_set_ret(fn, name);
    ast_node_set_end(fn, parser_file_p(f));
    return fn;

err_free:
    ast_node_destroy(fn);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
#endif
}
