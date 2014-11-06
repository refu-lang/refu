#ifndef LFR_ANALYZER_TYPECHECK_H
#define LFR_ANALYZER_TYPECHECK_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>

#include <stdbool.h>

struct symbol_table;
struct ast_node;
struct RFstring;

int analyzer_identifier_is_builtin(const struct RFstring *id);
bool analyzer_type_check(struct ast_node *type, struct symbol_table *st);

#endif
