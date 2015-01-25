#ifndef LFR_AST_BLOCK_DECLS_H
#define LFR_AST_BLOCK_DECLS_H

struct ast_node;
struct inplocation_mark;

#include <analyzer/symbol_table.h>

struct ast_block {
    //! The block's symbol table. Only initialized in the analyzer phase
    struct symbol_table st;
};
#endif
