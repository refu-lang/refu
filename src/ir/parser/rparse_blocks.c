#include <ir/parser/rirparser.h>

#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir_convert.h>
#include <ir/rir.h>

#include <Utils/sanity.h>

static struct rir_object *parse_assignment(struct rir_parser *p, struct token *tok, const struct RFstring *name)
{
    struct rir_object *retobj = NULL;
    rir_pctx_set_id(&p->ctx, name);
    switch (rir_toktype(tok)) {
    case RIR_TOK_CONVERT:
        retobj = rir_parse_convert(p);
        break;
    case RIR_TOK_READ:
        retobj = rir_parse_read(p);
        break;
    case RIR_TOK_CALL:
        retobj = rir_parse_call(p);
        break;
    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Unexpected rir token \""RF_STR_PF_FMT"\" after outer assignment to identifier.",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        break;
    }
    rir_pctx_reset_id(&p->ctx);
    return retobj;
}

static struct rir_object *rir_parse_label(struct rir_parser *p, struct rir_block *b, struct rirobj_strmap *map, const char *msg)
{
    struct token *tok;
    if (!(tok = lexer_expect_token(&p->lexer, RIR_TOK_IDENTIFIER_LABEL))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(&p->lexer),
            NULL,
            "Expected a branch label identifier %s.", msg
        );
        return NULL;
    }
    const struct RFstring *id = ast_identifier_str(tok->value.value.ast);
    struct rir_object *obj = strmap_get(map, id);
    if (!obj) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Parsed non-existing branch label "RF_STR_PF_FMT" %s.",
            RF_STR_PF_ARG(id),
            msg
        );
        return NULL;
    }
    return obj;
}

static bool rir_parse_branch(struct rir_parser *p, struct rir_block *b, struct rirobj_strmap *map)
{
    // consume 'branch'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_BRANCH))) {
        return false;
    }
    // get label argument
    struct rir_object *bobj = rir_parse_label(p, b, map, "at branch()");
    if (!bobj) {
        return false;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' after 'branch'.");
        return false;
    }
    rir_block_exit_init_branch(&b->exit, rir_object_block_label(bobj));
    return true;
}

static bool rir_parse_condbranch(struct rir_parser *p, struct rir_block *b, struct rirobj_strmap *map)
{
    // consume 'condbranch'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_CONDBRANCH))) {
        return false;
    }
    // get first value argument
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("condbranch() first argument");
    struct rir_value *cond = rir_parse_val_and_comma(p, &lmsg);
    if (!cond) {
        return false;
    }

    struct rir_object *objtrue = rir_parse_label(p, b, map, "at second argument of condbranch()");
    if (!objtrue) {
        return false;
    }
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ',' after second argument of 'condbranch()'.");
    }

    struct rir_object *objfalse = rir_parse_label(p, b, map, "at third argument of condbranch()");
    if (!objfalse) {
        return false;
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' after 'condbranch()'.");
        return false;
    }
    rir_block_exit_init_condbranch(&b->exit, cond, rir_object_block_label(objtrue), rir_object_block_label(objfalse));
    return true;
}

static bool rir_parse_return(struct rir_parser *p, struct rir_block *b)
{
    // consume 'return'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_RETURN))) {
        return false;
    }
    // get return value
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("at return()");
    struct rir_value *v = rir_parse_value(p, &lmsg);
    if (!v) {
        return false;
    }
    rir_block_exit_return_init(&b->exit, v);

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a ')' at the end of 'return'.");
        return false;
    }
    return true;
}

static bool rir_parse_block_expr(
    struct rir_parser *p,
    struct rir_block *b,
    struct rir_expression **retexpr,
    struct rirobj_strmap *map
)
{
    struct token *tok;
    if (!(tok = lexer_lookahead(&p->lexer, 1))) {
        return false;
    }

    struct rir_object *obj;
    switch(rir_toktype(tok)) {
    case RIR_TOK_IDENTIFIER_VARIABLE:
    {
        if (!(obj = rir_accept_identifier_var(p, tok, parse_assignment))) {
            RF_ERROR("Failed to parse an identifier assignment in a rir block");
            return false;
        }
        *retexpr = rir_object_to_expr(obj);
        return true;
    }
    case RIR_TOK_WRITE:
        if (!(obj = rir_parse_write(p))) {
            return false;
        }
        *retexpr = rir_object_to_expr(obj);
        return true;
    case RIR_TOK_RETURN:
        return rir_parse_return(p, b);
    case RIR_TOK_BRANCH:
        return rir_parse_branch(p, b, map);
    case RIR_TOK_CONDBRANCH:
        return rir_parse_condbranch(p, b, map);
    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Unexpected rir token \""RF_STR_PF_FMT"\" during parsing",
            RF_STR_PF_ARG(rir_tokentype_to_str(rir_toktype(tok)))
        );
        break;
    }
    return false;
}

static bool rir_parse_block(struct rir_parser *p, struct token *tok, struct rirobj_strmap *map)
{
    RF_ASSERT(rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL,
              "Expected a label identifier at the beginning.");
    const struct RFstring *id = ast_identifier_str(tok->value.value.ast);
    struct rir_object *obj = strmap_get(map, id);
    RF_ASSERT(obj, "Block name not found in map. Should not happen.");
    RF_ASSERT(obj->category == RIR_OBJ_BLOCK, "Should have a block here");
    struct rir_block *b = &obj->block;

    // consume the label identifier
    lexer_curr_token_advance(&p->lexer);

    struct rir_expression *expr;
    bool end_found = false;
    while (rir_parse_block_expr(p, b, &expr, map) &&
           !(end_found = b->exit.type != RIR_BLOCK_EXIT_INVALID)) {
        rir_block_add_expr(b, expr);
    }
    return end_found;
}

static bool rir_parse_create_basic_blocks(struct rir_parser *p, struct rirobj_strmap *map)
{
    struct token *tok;
    enum rir_token_type type;
    lexer_push(&p->lexer);
    strmap_init(map);
    while ((tok = lexer_lookahead(&p->lexer, 1)) &&
           (type = rir_toktype(tok)) != RIR_TOK_SM_CCBRACE) {
        if (type == RIR_TOK_IDENTIFIER_LABEL) {
            const struct RFstring *id = ast_identifier_str(tok->value.value.ast);
            struct rir_object *obj = strmap_get(map, id);
            if (!obj) {
                // if in this block we have not seen the destination label before, make a block
                obj = rir_block_create_obj(id, RIRPOS_PARSE, &p->ctx);
                if (!obj) {
                    RF_ERROR("Failed to create a rir block during parsing");
                    goto fail;
                }
                strmap_add(map, id, obj);
            }
        }
        // go to the next token
        lexer_curr_token_advance(&p->lexer);
    }
    lexer_rollback(&p->lexer);
    return true;

fail:
    rirobjmap_free(map, &p->ctx.common);
    return false;
}

bool rir_parse_bigblock(struct rir_parser *p, const char *position)
{
    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_OCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '{' after the %s.", position);
        return false;
    }

    struct rirobj_strmap map;
    if (!rir_parse_create_basic_blocks(p, &map)) {
        return false;
    }

    struct token *tok;
    while ((tok = lexer_lookahead(&p->lexer, 1)) &&
           rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL) {
        if (!rir_parse_block(p, tok, &map)) {
            goto fail_free_map;
        }
    }

    if (!lexer_expect_token(&p->lexer, RIR_TOK_SM_CCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(&p->lexer), NULL,
                         "Expected a '}' at the end of the block.");
        goto fail_free_map;
    }

    // success
    strmap_clear(&map); // just clear the map but keep the objects
    return true;

fail_free_map:
    rirobjmap_free(&map, &p->ctx.common);
    return false;
}
