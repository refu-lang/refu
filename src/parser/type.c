#include <parser/type.h>

#include <Utils/sanity.h>

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

enum tpar_state {
    TPAR_START = 0,
    TPAR_IDENTIFIER,
    TPAR_COLON,
    TPAR_TYPEOP
};

struct ast_node *parser_file_acc_typedesc(struct parser_file *f,
                                          int *paren_count)
{

    struct ast_node *id;
    struct ast_node *last_desc = NULL;
    struct ast_node *last_op = NULL;
    struct ast_node *last_id = NULL;
    struct parser_offset proff;
    char *sp;
    char *ep;
    enum typeop_type optype;
    enum tpar_state state = TPAR_START;

    parser_offset_copy(&proff, &f->offset);

    do {
        parser_file_acc_ws(f);
        switch (state) {
        case TPAR_START:
            id = parser_file_acc_identifier(f);
            if (id) {
                //got an identifier
                last_id = id;
                state = TPAR_IDENTIFIER;
            } else {
                //no type description found
                goto end;
            }
            break;
        case TPAR_IDENTIFIER:
            if (parser_file_acc_string_ascii(f, &parser_tok_colon)) {
                //got a colon
                state = TPAR_COLON;
            } else if ((optype = parser_file_acc_typeop_token(f)) != TYPEOP_INVALID) {
                //got a type operator
                state = TPAR_TYPEOP;
            } else if (last_id == NULL) {
                //we are done
                return last_op ? last_op : last_desc;
            } else {
                //error
                parser_file_synerr(
                    f,
                    "Expected either a ':' or a type operator"
                );
                goto end;
            }
            break;
        case TPAR_COLON:
            id = parser_file_acc_identifier(f);
            if (id) {
                // IDENTIFIER ':' IDENTIFIER -- > type description
                last_desc = ast_typedesc_create(f,
                                                ast_node_startsp(last_id),
                                                ast_node_endsp(id),
                                                last_id,
                                                id);

                // if we had a dangling type operator
                if (last_op) {
                    ast_typeop_set_right(&last_op->typeop, last_desc);
                    ast_node_set_end(last_op, ast_node_endsp(last_desc));

                    last_op = NULL;
                    last_desc = last_op;
                }

                state = TPAR_IDENTIFIER;
                last_id = NULL;
            } else {
                //error
                parser_file_synerr(
                    f,
                    "Expected an identifier after ':'"
                );
                goto end;
            }
            break;
        case TPAR_TYPEOP:
            id = parser_file_acc_identifier(f);
            if (id) {
                last_op = ast_typeop_create(f,
                                            ast_node_startsp(last_desc),
                                            NULL, //end is not known yet
                                            optype,
                                            last_desc,
                                            NULL); //end is not known yet
                last_id = id;
                //got an identifier
                state = TPAR_IDENTIFIER;
            } else {
                //error
                parser_file_synerr(
                    f,
                    "Expected an identifier after a type operator"
                );
                goto end;
            }
            break;
        default:
            RF_ASSERT(0);
            break;
        }
    } while(1);

end:
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
    ast_node_destroy(data_decl);
not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}
