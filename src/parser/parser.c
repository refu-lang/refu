#include <parser/parser.h>
#include <parser/parser_common.h>

#include <rfbase/utils/memory.h>

#include <info/info.h>
#include <ast/ast.h>

i_INLINE_INS struct ast_parser *parser_common_to_astparser(const struct parser_common* c);

bool ast_parser_init(
    struct ast_parser *p,
    struct inpfile *file,
    struct lexer *lex,
    struct info_ctx *info,
    struct front_ctx *front
)
{
    RF_STRUCT_ZERO(p);
    parser_common_init(&p->cmn, PARSER_AST, front, file, lex, info);
    p->have_syntax_err = false;
    return true;
}

struct ast_parser *ast_parser_create(
    struct inpfile *f,
    struct lexer *lex,
    struct info_ctx *info,
    struct front_ctx *front
)
{
    struct ast_parser *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!ast_parser_init(ret, f, lex, info, front)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void ast_parser_deinit(struct ast_parser *p)
{
    if (p->root) {
        // if parser has ownership of ast tree
        ast_node_destroy(p->root);
    }
    parser_common_deinit(&p->cmn);
}

void ast_parser_destroy(struct ast_parser *p)
{
    ast_parser_deinit(p);
    free(p);
}

i_INLINE_INS void ast_parser_set_syntax_error(struct ast_parser *parser);
i_INLINE_INS bool ast_parser_has_syntax_error(struct ast_parser *parser);
i_INLINE_INS bool ast_parser_has_syntax_error_reset(struct ast_parser *parser);

void ast_parser_info_rollback(struct ast_parser *parser)
{
    info_ctx_rollback(parser->cmn.info);
    parser->have_syntax_err = false;
}
