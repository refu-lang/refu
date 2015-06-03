#include "import.h"

#include <parser/parser.h>
#include <lexer/lexer.h>

#include <ast/import.h>

#include "common.h"
#include "identifier.h"

struct ast_node *parser_acc_import(struct parser *p)
{
    struct ast_node *id;
    struct ast_node *import;
    const struct inplocation_mark *end = NULL;
    lexer_push(p->lexer);
    struct token *tok = lexer_lookahead(p->lexer, 1);
    if (!tok && (tok->type != TOKEN_KW_IMPORT || tok->type != TOKEN_KW_FOREIGN_IMPORT)) {
        return NULL;
    }
    // consume import
    lexer_next_token(p->lexer);

    import = ast_import_create(token_get_start(tok), NULL, tok->type == TOKEN_KW_FOREIGN_IMPORT);
    if (!import) {
        goto fail;
    }

    do {
        id = parser_acc_identifier(p);
        if (!id) {
            parser_synerr(
                p,
                lexer_last_token_end(p->lexer),
                NULL,
                ast_import_is_foreign(import)
                    ? "Expected an identifier at foreign_import statement"
                    : "Expected an identifier at import statement"
            );
            goto fail_free;
        }
        ast_node_add_child(import, id);
        end = ast_node_endmark(id);
    } while (lexer_expect_token(p->lexer, TOKEN_OP_COMMA));
    ast_node_set_end(import, end);

    lexer_pop(p->lexer);
    return import;

fail_free:
    ast_node_destroy(import);
fail:
    lexer_rollback(p->lexer);
    return NULL;
}
