#ifndef LFR_SERIALIZER_ASTPRINTER_H
#define LFR_SERIALIZER_ASTPRINTER_H

#include <stdio.h>
#include <stdbool.h>

struct ast_node;

/**
 * Prints the ast in json format in the specified file stream (can be stdout)
 */
bool ast_output_to_file(const struct ast_node *root, FILE *f);

#endif
