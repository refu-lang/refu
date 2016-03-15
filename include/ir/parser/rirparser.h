#ifndef LFR_IR_RIR_PARSER_H
#define LFR_IR_RIR_PARSER_H

#include <rflib/string/core.h>

#include <lexer/lexer.h>
#include <ir/rir.h>
#include <ir/rir_expression.h>
#include <parser/parser_common.h>

struct inpfile;
struct info_ctx;
struct rir_type_arr;
struct value_arr;

/**
 * Context passed around to rir functions when parsing rir files from text
 * to create a RIR.
 *
 * For converting from ast @see rir_ctx
 */
struct rir_pctx {
    //! Context data common to both ast and parsing rir code
    //! @warning Always keep first
    struct rir_common common;
    //! Some functions from the parsing require an id string as input.
    const struct RFstring *id;
    //! Denotes if during parsing a module name was found and a module was created.
    //! If not then this rir module is by default made the main module
    bool module_created;
};

i_INLINE_DECL void rir_pctx_init(struct rir_pctx *ctx, struct rir *r)
{
    RF_STRUCT_ZERO(ctx);
    ctx->common.rir = r;
}

i_INLINE_DECL void rir_pctx_set_id(struct rir_pctx *ctx, const struct RFstring *id)
{
    RF_ASSERT(ctx->id == NULL, "Context string ID should be empty");
    ctx->id = id;
}

i_INLINE_DECL void rir_pctx_reset_id(struct rir_pctx *ctx)
{
    RF_ASSERT(ctx->id != NULL, "Context string ID should not be empty");
    ctx->id = NULL;
}

struct rir_parser {
    //! The parser common data. Should always be first. Some behaviour relies on that.
    struct parser_common cmn;
    //! The buffer for the parsed string
    struct RFstringx buff;
    //! The rir parsing context
    struct rir_pctx ctx;
};

i_INLINE_DECL struct rir_parser *parser_common_to_rirparser(const struct parser_common* c)
{
    RF_ASSERT(c->type == PARSER_RIR, "Expected RIR parser");
    return container_of(c, struct rir_parser, cmn);
}

struct rir_parser *rir_parser_create(
    struct front_ctx *front,
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *ctx
);
bool rir_parser_init(
    struct rir_parser *p,
    struct front_ctx *front,
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *ctx
);
void rir_parser_destroy(struct rir_parser *p);
void rir_parser_deinit(struct rir_parser *p);


bool rir_parse(struct rir_parser *p);

#define rirparser_synerr(parser_, start_, end_, ...)    \
    do {                                                \
        i_info_ctx_add_msg((parser_)->cmn.info,         \
                           MESSAGE_SYNTAX_ERROR,        \
                           (start_),                    \
                           (end_),                      \
                           __VA_ARGS__);                \
    } while(0)


i_INLINE_DECL struct rir *rir_parser_rir(const struct rir_parser *p)
{
    return p->ctx.common.rir;
}

bool rir_parse_bigblock(struct rir_parser *p, const char *position);
struct rir_value *rir_parse_value(struct rir_parser *p, const struct RFstring *msg);
bool rir_parse_valuearr(struct rir_parser *p, struct value_arr *arr, const struct RFstring *msg);
struct rir_type *rir_parse_type(struct rir_parser *p, const struct RFstring *msg);
bool rir_parse_typearr(
    struct rir_parser *p,
    struct rir_type_arr *arr,
    enum rir_token_type ending_token
);
struct rir_object *rir_parse_global(struct rir_parser *p, const struct RFstring *name);
struct rir_object *rir_accept_identifier_var(
    struct rir_parser *p,
    struct token *tok,
    struct rir_object *(*assignment_parser)(struct rir_parser*, struct token*, const struct RFstring *name)
);
struct rir_object *rir_parse_typedef(
    struct rir_parser *p,
    const struct RFstring *name,
    bool uniondef
);
bool rir_parse_fndef(struct rir_parser *p);
bool rir_parse_fndecl(struct rir_parser *p);
struct rir_object *rir_parse_convert(struct rir_parser *p);
struct rir_object *rir_parse_write(struct rir_parser *p);
struct rir_object *rir_parse_read(struct rir_parser *p);
struct rir_object *rir_parse_call(struct rir_parser *p);
struct rir_object *rir_parse_binaryop(
    struct rir_parser *p,
    enum rir_token_type type
);

/* -- util rir parsing functions -- */
/**
 * Consume a command token and the following parentheses.
 *
 * @param p        The parser instance
 * @param msg      Description of the token to display in case of failure.
 */
bool rir_parse_instr_start(struct rir_parser *p, const struct RFstring *msg);
struct rir_value *rir_parse_val_and_comma(struct rir_parser *p, const struct RFstring *msg);
struct rir_type *rir_parse_type_and_comma(struct rir_parser *p, const struct RFstring *msg);
#endif
