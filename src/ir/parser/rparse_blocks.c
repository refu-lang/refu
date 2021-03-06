#include <ir/parser/rirparser.h>

#include <rfbase/utils/sanity.h>

#include <lexer/lexer.h>
#include <ir/rir_function.h>
#include <ir/rir_object.h>
#include <ir/rir_convert.h>
#include <ir/rir.h>

static struct rir_object *parse_assignment(
    struct rir_parser *p,
    struct token *tok,
    const struct RFstring *name
)
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
    case RIR_TOK_ADD:
    case RIR_TOK_SUB:
    case RIR_TOK_MUL:
    case RIR_TOK_DIV:
    case RIR_TOK_CMPEQ:
    case RIR_TOK_CMPNE:
    case RIR_TOK_CMPGT:
    case RIR_TOK_CMPGE:
    case RIR_TOK_CMPLT:
    case RIR_TOK_CMPLE:
        retobj = rir_parse_binaryop(p, rir_toktype(tok));
        break;

    default:
        rirparser_synerr(
            p,
            token_get_start(tok),
            NULL,
            "Unexpected rir token \""RFS_PF"\" after outer assignment to identifier.",
            RFS_PA(rir_tokentype_to_str(rir_toktype(tok)))
        );
        break;
    }
    rir_pctx_reset_id(&p->ctx);
    return retobj;
}

static struct rir_object *rir_parse_label(
    struct rir_parser *p,
    struct rir_block *b,
    struct rirobj_strmap *map,
    const char *msg
)
{
    struct token *tok;
    if (!(tok = lexer_expect_token(parser_lexer(p), RIR_TOK_IDENTIFIER_LABEL))) {
        rirparser_synerr(
            p,
            lexer_last_token_start(parser_lexer(p)),
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
            "Parsed non-existing branch label "RFS_PF" %s.",
            RFS_PA(id),
            msg
        );
        // going out with obj == NULL so clean error path
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

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
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
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_COMMA)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ',' after second argument of 'condbranch()'.");
    }

    struct rir_object *objfalse = rir_parse_label(p, b, map, "at third argument of condbranch()");
    if (!objfalse) {
        return false;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' after 'condbranch()'.");
        return false;
    }
    rir_block_exit_init_condbranch(&b->exit, cond, rir_object_block_label(objtrue), rir_object_block_label(objfalse));
    return true;
}

static bool rir_parse_return(struct rir_parser *p, struct rir_block *b)
{
    struct rir_value *v = NULL;
    // consume 'return'
    if (!rir_parse_instr_start(p, rir_tokentype_to_str(RIR_TOK_RETURN))) {
        return false;
    }

    struct token *tok;
    if (!(tok = lexer_lookahead(parser_lexer(p), 1))) {
        rirparser_synerr(
            p,
            token_get_start(tok),
            token_get_end(tok),
            "Expected either a value or a ')' at 'return'."
        );
        return false;
    }

    if (rir_toktype(tok) == RIR_TOK_SM_CPAREN) { // if a ')' return has no val
        lexer_curr_token_advance(parser_lexer(p));
        goto success;
    }

    // get return value
    static const struct RFstring lmsg = RF_STRING_STATIC_INIT("at return()");
    if (!(v = rir_parse_value(p, &lmsg))) {
        return false;
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CPAREN)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a ')' at the end of 'return'.");
        return false;
    }

success:
    rir_block_exit_return_init(&b->exit, v);
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
    if (!(tok = lexer_lookahead(parser_lexer(p), 1))) {
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
            "Unexpected rir token \""RFS_PF"\" during parsing",
            RFS_PA(rir_tokentype_to_str(rir_toktype(tok)))
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
    lexer_curr_token_advance(parser_lexer(p));

    struct rir_expression *expr;
    bool end_found = false;
    while (rir_parse_block_expr(p, b, &expr, map) &&
           !(end_found = b->exit.type != RIR_BLOCK_EXIT_INVALID)) {
        rir_block_add_expr(b, expr);
    }
    // at the end make sure the block is part of the current function
    rir_fndef_add_block(rir_data_curr_fn(&p->ctx), b);
    return end_found;
}

static bool rir_parse_create_basic_blocks(struct rir_parser *p, struct rirobj_strmap *map)
{
    struct token *tok;
    enum rir_token_type type;
    lexer_push(parser_lexer(p));
    strmap_init(map);
    while ((tok = lexer_lookahead(parser_lexer(p), 1)) &&
           (type = rir_toktype(tok)) != RIR_TOK_SM_CCBRACE) {
        if (type == RIR_TOK_IDENTIFIER_LABEL) {
            struct RFstring *id = (struct RFstring*)ast_identifier_str(tok->value.value.ast);
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
        lexer_curr_token_advance(parser_lexer(p));
    }
    lexer_rollback(parser_lexer(p));
    return true;

fail:
    rirobjmap_free(map, &p->ctx.common);
    return false;
}

bool rir_parse_bigblock(struct rir_parser *p, const char *position)
{
    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_OCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
                         "Expected a '{' after the %s.", position);
        return false;
    }

    struct rirobj_strmap map;
    if (!rir_parse_create_basic_blocks(p, &map)) {
        return false;
    }

    struct token *tok;
    while ((tok = lexer_lookahead(parser_lexer(p), 1)) &&
           rir_toktype(tok) == RIR_TOK_IDENTIFIER_LABEL) {
        if (!rir_parse_block(p, tok, &map)) {
            goto fail_free_map;
        }
    }

    if (!lexer_expect_token(parser_lexer(p), RIR_TOK_SM_CCBRACE)) {
        rirparser_synerr(p, lexer_last_token_start(parser_lexer(p)), NULL,
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
