#include "vardecl.h"


#include <ast/ast.h>
#include <ast/vardecl.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include "common.h"
#include "type.h"
#include "expression.h"

// parsing a variable declaration is basically like a type leaf
struct ast_node *ast_parser_acc_vardecl(struct ast_parser *p)
{
    lexer_push(parser_lexer(p));

    struct ast_node *desc = ast_parser_acc_typeleaf(p);
    if (!desc) {
        goto fail;
    }

    struct ast_node *n = ast_vardecl_create(
        ast_node_startmark(desc),
        ast_node_endmark(desc),
        desc
    );
    if (!n) {
        RF_ERRNOMEM();
        goto fail;
    }

    lexer_pop(parser_lexer(p));
    return n;

fail:
    lexer_rollback(parser_lexer(p));
    return NULL;
}
