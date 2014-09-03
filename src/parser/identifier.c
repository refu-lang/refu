#include <ast/ast.h>
#include <ast/identifier.h>

#include <parser/identifier.h>
#include <parser/generics.h>

#define COND_IDENTIFIER_BEGIN(p_)               \
    (((p_) >= 'A' && (p_) <= 'Z') ||            \
     ((p_) >= 'a' && (p_) <= 'z'))

#define COND_IDENTIFIER(p_)                     \
    (COND_IDENTIFIER_BEGIN(p_) ||               \
     ((p_) >= '0' && (p_) <= '9'))

struct ast_node *parser_file_acc_identifier(struct parser_file *f)
{
    struct parser_offset proff;
    char *p;
    char *sp;
    char *ep;
    char *lim;
    bool last_char_relevant = false;

    parser_offset_copy(&proff, &f->offset);

    parser_file_acc_ws(f);
    sp = p = parser_file_sp(f);
    lim = parser_file_sp(f) + rf_string_length_bytes(parser_file_str(f)) - 1;

    if (lim - p <= 0) { /* if already at the end do nothing */
        goto not_found;
    }

    if (COND_IDENTIFIER_BEGIN(*p)) {
        if (p < lim) { /* don't go over the limit */
            if (COND_IDENTIFIER((*(p+1)))) {
                p ++;
            } else {
                goto end;
            }
        } else {
            last_char_relevant = true;
            goto end;
        }
    } else {
        goto not_found;
    }

    while (p <= lim) {
        if (COND_IDENTIFIER(*p)) {
            if (p < lim) { /* don't go over the limit */
                if (COND_IDENTIFIER((*(p+1)))) {
                    p ++;
                } else {
                    break;
                }
            } else {
                last_char_relevant = true;
                break;
            }
            continue;
        }
        break;
    }

    if (p == sp) { /* no identifier was found */
        goto not_found;
    }

end:
    if (p == lim && !last_char_relevant) {
        p --;
    }
    ep = p;

    parser_file_move(f, p - sp + 1, p - sp + 1);
    return ast_identifier_create(f, sp, ep);

not_found:
    parser_file_move_to_offset(f, &proff);
    return NULL;
}

struct ast_node *parser_file_acc_xidentifier(struct parser_file *f)
{
    struct parser_offset proff;
    char *sp;
    struct ast_node *id;
    struct ast_node *xid;
    struct ast_node *genr;
    bool is_const = false;
    parser_offset_copy(&proff, &f->offset);

    PARSER_CHECK_EOF(parser_file_acc_ws(f), f, goto not_found);
    sp = parser_file_sp(f);
    // parsing logic for the annotations to the identifier here
    if (parser_file_acc_string_ascii(f, &parser_kw_const)) {
        is_const = true;
    }
    id = parser_file_acc_identifier(f);
    if (!id) {
        goto not_found;
    }
    genr = parser_file_acc_genrattr(f);
    if (parser_file_has_synerr(f)) {
        ast_node_destroy(id);
        goto not_found;
    }
    
    xid = ast_xidentifier_create(f, sp, ast_node_endsp(id), id, is_const, genr);
    if (!xid) {
        //TODO: error
        goto not_found;
    }

    return xid;
not_found:
parser_file_move_to_offset(f, &proff);
return NULL;
}
