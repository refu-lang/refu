#include <ir/parser/rirparser.h>
#include <Utils/memory.h>
#include <String/rf_str_corex.h>

#include <inpfile.h>

#include <ir/rir.h>

static bool rir_parser_parse(struct rir_parser *p);

struct rir_parser *rir_parser_create()
{
    struct rir_parser *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    if (!rir_parser_init(ret)) {
        free(ret);
        return NULL;
    }
    return ret;
}

bool rir_parser_init(struct rir_parser *p)
{
    // initialize the parsing buffer
    if (!rf_stringx_init_buff(&p->buff, 1024, "")) {
        return false;
    }

    return true;
}

void rir_parser_destroy(struct rir_parser *p)
{
    rir_parser_deinit(p);
    free(p);
}

void rir_parser_deinit(struct rir_parser *p)
{
    rf_stringx_deinit(&p->buff);
}

bool rir_parser_parse_file(struct rir_parser *p, const struct RFstring *name)
{
    if (!(p->file = inpfile_create(name))) {
        RF_ERROR("Failed to parse input rir file");
        return false;
    }
    return rir_parser_parse(p);
}

bool rir_parser_parse_string(struct rir_parser *p, const struct RFstring *name, struct RFstring *str)
{
    if (!(p->file = inpfile_create_from_string(name, str))) {
        RF_ERROR("Failed to parse input rir file");
        return false;
    }
    return rir_parser_parse(p);
}

static bool rir_parser_parse(struct rir_parser *p)
{
    if (!lexer_init(&p->lexer, p->file, NULL /* TODO: Fill this in correctly */, true)) {
        return false;
    }
    if (!lexer_scan(&p->lexer)) {
        return false;
    }

    struct rir *r = rir_create();

    do {
        // TODO
    } while (!inpfile_at_eof(p->file));

    // success
    return true;
    
/* fail_free_rir: */
    rir_destroy(r);
    return false;
}
