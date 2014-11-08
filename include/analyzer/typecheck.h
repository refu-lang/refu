#ifndef LFR_ANALYZER_TYPECHECK_H
#define LFR_ANALYZER_TYPECHECK_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>

#include <stdbool.h>

struct analyzer;
struct RFstring;
struct type;
struct ast_node;

bool analyzer_typecheck(struct analyzer *a);

const struct type *expression_determine_type(struct ast_node *expr);

#endif
