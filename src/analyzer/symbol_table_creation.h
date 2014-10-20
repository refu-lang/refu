#ifndef LFR_ANALYZER_SYMBOL_TABLE_CREATION_H
#define LFR_ANALYZER_SYMBOL_TABLE_CREATION_H

#include <stdbool.h>

struct analyzer;

/**
 * Initializes the symbol tables for the nodes where this is needed and also
 * populates them with values. Since this is the first pass of the analyzer
 * phase this function also changes the ownership of the ast nodes.
 *
 * @param a     The analyzer handle
 * @return      True for success, false otherwise
 */
bool analyzer_create_symbol_tables(struct analyzer *a);

#endif
