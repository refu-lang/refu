#ifndef LFR_ANALYZER_SYMBOL_TABLE_CREATION_H
#define LFR_ANALYZER_SYMBOL_TABLE_CREATION_H

#include <stdbool.h>

struct analyzer;
bool analyzer_populate_symbol_tables(struct analyzer *a);

#endif
