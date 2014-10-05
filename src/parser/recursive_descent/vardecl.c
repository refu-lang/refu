#include "vardecl.h"


#include <ast/ast.h>
#include <ast/vardecl.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include "common.h"
#include "type.h"

struct ast_node *parser_acc_vardecl(struct parser *p)
{
    struct ast_node *n;
    struct token *tok;
    struct token *tok2;
    struct ast_node *name;
    struct ast_node *type;

    tok = lexer_lookahead(p->lexer, 1);
    tok2 = lexer_lookahead(p->lexer, 2);
    if (!TOKENS_ARE_POSSIBLE_VARDECL(tok, tok2)) {
        return NULL;
    }

    // don't think this can't fail since tok is known to be an identifier here
    name = token_get_value(tok);
    //consume identifier and ':'
    lexer_next_token(p->lexer);
    lexer_next_token(p->lexer);

    type = parser_acc_typedesc(p);
    if (!type) {
        parser_synerr(p, token_get_end(tok2), NULL,
                      "expected type description on the right of ':'");
        ast_node_destroy(name);
        return NULL;
    }

    n = ast_vardecl_create(ast_node_startmark(name), ast_node_endmark(type),
                           name, type);
    if (!n) {
        RF_ERRNOMEM();
        ast_node_destroy(name);
        ast_node_destroy(type);
        return NULL;
    }

    return n;
}
