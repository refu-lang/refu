#include <parser/parser_common.h>

#include <parser/parser.h>
#include <ir/parser/rirparser.h>

i_INLINE_INS void parser_common_init(struct parser_common *c, enum parser_type, struct inpfile *f, struct lexer *lexer, struct info_ctx *info);
i_INLINE_INS void parser_common_deinit(struct parser_common *c);

void parser_destroy(struct parser_common *c)
{
    switch (c->type) {
    case PARSER_AST:
        ast_parser_destroy(parser_common_to_astparser(c));
        break;
    case PARSER_RIR:
        rir_parser_destroy(parser_common_to_rirparser(c));
        break;
    default:
        RF_CRITICAL_FAIL("Illegal parser type");
        break;
    }
}

void parser_common_flush_messages(struct parser_common *c)
{
    info_ctx_flush(c->info, stdout, MESSAGE_ANY);
}

bool parser_parse(struct parser_common *c)
{
    switch (c->type) {
    case PARSER_AST:
        return ast_parser_process_file(parser_common_to_astparser(c));
    case PARSER_RIR:
        return rir_parse(parser_common_to_rirparser(c));
    default:
        RF_CRITICAL_FAIL("Illegal parser type");
        break;
    }
    return false;
}

struct ast_node *parser_ast_get_root(struct parser_common *c)
{
    RF_ASSERT(c->type == PARSER_AST, "Expected ast parser");
    return parser_common_to_astparser(c)->root;
}

struct ast_node *parser_ast_move_root(struct parser_common *c)
{
    RF_ASSERT(c->type == PARSER_AST, "Expected ast parser");
    struct ast_node *ret = parser_common_to_astparser(c)->root;
    parser_common_to_astparser(c)->root = NULL;
    return ret;
}

struct rir *parser_rir(struct parser_common *c)
{
    RF_ASSERT(c->type == PARSER_RIR, "Expected rir parser");
    return rir_parser_rir(parser_common_to_rirparser(c));
}
