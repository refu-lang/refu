#ifndef LFR_ANALYZER_GLOBAL_CONTEXT_H
#define LFR_ANALYZER_GLOBAL_CONTEXT_H

#include <stdbool.h>
struct analyzer;

/**
 * Loads the global context to the analyzer
 *
 * Global context is basically all built-in or special keywords/functions e.t.c.
 */
bool analyzer_load_globals(struct analyzer *a);
#endif
