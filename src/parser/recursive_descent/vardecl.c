#include "vardecl.h"


#include <ast/ast.h>
#include <ast/vardecl.h>
#include <ast/identifier.h>
#include <info/info.h>

#include <parser/parser.h>
#include <lexer/lexer.h>
#include "common.h"
#include "type.h"

// parsing a variable declaration is basically like a type leaf
struct ast_node *parser_acc_vardecl(struct parser *p)
{
    struct ast_node *n;
    struct ast_node *desc;

    desc = parser_acc_typeleaf(p);
    if (!desc) {
        return NULL;
    }

    n = ast_vardecl_create(ast_node_startmark(desc), ast_node_endmark(desc),
                           desc);
    if (!n) {
        RF_ERRNOMEM();
        return NULL;
    }

    return n;
}
