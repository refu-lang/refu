#include <parser/parser.h>

#include <Utils/memory.h>

#include <info/info.h>

bool parser_init(struct parser *p,
                 struct inpfile *file,
                 struct lexer *lex,
                 struct info_ctx *info)
{

    p->file = file;
    p->lexer = lex;
    p->info = info;
    return true;
}

struct parser *parser_create(struct inpfile *f,
                             struct lexer *lex,
                             struct info_ctx *info)
{
    struct parser *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!parser_init(ret, f, lex, info)) {
        free(ret);
        return NULL;
    }
    return ret;
}

void parser_deinit(struct parser *p)
{
    p->lexer = NULL;
    p->info = NULL;
    p->file = NULL;
}

void parser_destroy(struct parser *p)
{
    free(p);
}

void parser_flush_messages(struct parser *p)
{
    info_ctx_flush(p->info, stdout, MESSAGE_ANY);
}
