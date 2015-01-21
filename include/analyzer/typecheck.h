#ifndef LFR_ANALYZER_TYPECHECK_H
#define LFR_ANALYZER_TYPECHECK_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>

#include <stdbool.h>

struct analyzer;
struct ast_node;
bool analyzer_typecheck(struct analyzer *a, struct ast_node *root);

#endif
