#ifndef LFR_IR_RIR_PARSER_H
#define LFR_IR_RIR_PARSER_H

#include <String/rf_str_core.h>
#include <lexer/lexer.h>
#include <ir/rir.h>

struct inpfile;
struct info_ctx;
struct rir_type_arr;

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
};

i_INLINE_DECL void rir_pctx_init(struct rir_pctx *ctx, struct rir *r)
{
    RF_STRUCT_ZERO(ctx);
    ctx->common.rir = r;
}

struct rir_parser {
    //! The buffer for the parsed string
    struct RFstringx buff;
    //! The input file representation
    struct inpfile *file;
    //! The lexer part of the parser
    struct lexer lexer;
    //! Pointer to the common info context
    struct info_ctx *info;
    //! The rir parsing context
    struct rir_pctx ctx;
};

struct rir_parser *rir_parser_create(const struct RFstring *name,
                                     const struct RFstring *contents);
bool rir_parser_init(struct rir_parser *p,
                     const struct RFstring *name,
                     const struct RFstring *contents);
void rir_parser_destroy(struct rir_parser *p);
void rir_parser_deinit(struct rir_parser *p);


bool rir_parse(struct rir_parser *p);

#define rirparser_synerr(parser_, start_, end_, ...) \
    do {                                          \
        i_info_ctx_add_msg((parser_)->info,       \
                           MESSAGE_SYNTAX_ERROR,  \
                           (start_),              \
                           (end_),                \
                           __VA_ARGS__);          \
    } while(0)


struct rir_value *rir_parse_value(struct rir_parser *p, struct rir *r, const char *msg);
struct rir_type *rir_parse_type(struct rir_parser *p, struct rir *r, const char *msg);
bool rir_parse_typearr(struct rir_parser *p, struct rir_type_arr *arr, struct rir *r);
bool rir_parse_global(struct rir_parser *p, struct token *tok, struct rir *r);
struct rir_object *rir_accept_identifier_var(
    struct rir_parser *p,
    struct token *tok,
    struct rir_object *(*assignment_parser)(struct rir_parser*, struct token*, const struct RFstring *name, struct rir*),
    struct rir *r
);
struct rir_object *rir_parse_typedef(
    struct rir_parser *p,
    const struct RFstring *name,
    bool uniondef,
    struct rir *r
);
bool rir_parse_fndef(struct rir_parser *p, struct rir *r);
bool rir_parse_fndecl(struct rir_parser *p, struct rir *r);
#endif
