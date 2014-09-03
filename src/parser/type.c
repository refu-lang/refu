#include <parser/type.h>

#include <Utils/sanity.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <parser/tokens.h>
#include <parser/identifier.h>



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

/**
 * Type description parsing state machine diagram.
 * Attempt to explain the implementation of the following function
 *
 *                  +-----+
 *                  |Start|
 *                  +--+--+
 *                     |
 *                     |
 *     got identifier  |   got open parentheses
 *           +---------+------------+
 *           |                      |
 *           |                      |
 *      +----v-----+         +------v-------+
 *      |          |         |              <------+
 *  +--->TPAR_LEFT |    +--> |  TPAR_OPAREN |      |
 *  |   +----+-----+    |    +---+----------+      |
 *  |        |colon     |        |    make         |
 *  |        |          |open    | new type desc   |
 *  |   +----v------+   |paren   |                 |
 *  |   |           |   |    +---v--+              | got open parentheses
 *  |   |TPAR_COLON +---+    |      |              |
 *  |   +----+------+        | NEW  |              |
 *  |        |               +--+---+              |
 *  |        |got xidentifier   |                  |
 *  |   +----v------+           |    + ------------+-+
 *  |   |           <-----------+    |               |
 *  |   |TPAR_RIGHT +-----------+---->  TPAR TYPEOP  |
 *  |   +----+------+ got typeop     +------------+--+
 *  |        |                                    |
 *  |        |                                    |
 *  |        |     +-------------+                | got identifier
 *  |        |     |             |                |
 *  |        +---> |TPAR CPAREN  |                |
 *  |        |     +-----+-------+                |
 *  |        |got ')'    | once ')' finish        |
 *  |        |           | current typedesc       |
 *  |        |           v                        |
 *  |        |       +-----+                      |
 *  |        +-----> |End  |                      |
 *  |   else finish  +-----+                      |
 *  |                                             |
 *  +---------------------------------------------+
*/
enum tpar_state {
    TPAR_START = 0,
    TPAR_OPAREN,
    TPAR_CPAREN,
    TPAR_LEFT,
    TPAR_COLON,
    TPAR_RIGHT,
    TPAR_TYPEOP,
    TPAR_END,
};
struct ast_node *parser_file_acc_typedesc(struct parser_file *f,
                                          int *paren_count)
{
    struct ast_node *n;
    struct ast_node *last_desc = NULL;
    struct ast_node *last_op = NULL;
    struct ast_node *last_id = NULL;
    struct ast_node *ret = NULL;
    struct parser_offset proff;
    enum typeop_type optype;
    enum tpar_state state = TPAR_START;

    parser_offset_copy(&proff, &f->offset);

    do {
        parser_file_acc_ws(f);
        switch (state) {
        case TPAR_START:
            if ((n = parser_file_acc_identifier(f))) {
                // got an identifier
                last_id = n;
                state = TPAR_LEFT;
            } else if (parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
                // opening parentheses right from the start
                state = TPAR_OPAREN;
            } else {
                //no type description found
                goto end;
            }
            break;
        case TPAR_OPAREN:
            *paren_count += 1;
            last_desc = parser_file_acc_typedesc(f, paren_count);
            if (!last_desc) {
                goto end;
            }
            ret = last_desc;
            state = TPAR_RIGHT;
            break;
        case TPAR_CPAREN:
            *paren_count -= 1;
            state = TPAR_END;
            break;
        case TPAR_END:
            if (!ret) {
                RF_ASSERT(0); // should not happen?
            }
            return ret;
            break;
        case TPAR_LEFT:
            if (parser_file_acc_string_ascii(f, &parser_tok_colon)) {
                //got a colon
                state = TPAR_COLON;
            } else {
                //error
                parser_file_synerr(f, 0, "Expected a ':'");
                goto end;
            }
            break;
        case TPAR_COLON:
            if ((n = parser_file_acc_xidentifier(f))) {
                // create the type description to send to TPAR_RIGHT
                last_desc = ast_typedesc_create(f,
                                                ast_node_startsp(last_id),
                                                ast_node_endsp(n),
                                                last_id,
                                                n);
                ret = last_desc;

                // if we had a dangling type operator
                if (last_op) {
                    ast_typeop_set_right(last_op, last_desc);
                    last_desc = last_op;
                    ret = last_op;

                    last_op = NULL;
                }

                state = TPAR_RIGHT;
                last_id = NULL;
            } else if (parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
                state = TPAR_OPAREN;
            } else {
                //error
                parser_file_synerr(
                    f, 0,
                    "Expected an identifier or a '(' after ':'"
                );
                goto end;
            }
            break;
        case TPAR_RIGHT:
            if ((optype = parser_file_acc_typeop_token(f)) != TYPEOP_INVALID) {
                state = TPAR_TYPEOP;
            } else if (parser_file_acc_string_ascii(f, &parser_tok_cparen)) {
                state = TPAR_CPAREN;
            } else {
                state = TPAR_END;
            }
            break;
        case TPAR_TYPEOP:
            last_op = ast_typeop_create(f,
                                        ast_node_startsp(last_desc),
                                        NULL, //end is not known yet
                                        optype,
                                        last_desc,
                                        NULL); //end is not known yet
            last_desc = NULL;

            if (optype == TYPEOP_SUM &&
                (n = parser_file_acc_typedesc(f, paren_count))) {
                ast_typeop_set_right(last_op, n);
                ret = last_op;
                state = TPAR_RIGHT;
            } else if ((n = parser_file_acc_identifier(f))) {
                last_id = n;
                state = TPAR_LEFT;
            } else if (parser_file_acc_string_ascii(f, &parser_tok_oparen)) {
                state = TPAR_OPAREN;
            } else {
                //error
                parser_file_synerr(
                    f, 0,
                    "Expected an identifier or '(' after a type operator"
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
    if (last_id) {
        ast_node_destroy(last_id);
    }
    if (last_desc) {
        ast_node_destroy(last_desc);
    }
    if (last_op) {
        ast_node_destroy(last_op);
    }
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
        parser_file_synerr(f, 0,
                           "Expected data description for data declaration "
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
        parser_file_synerr(f, 0,
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
