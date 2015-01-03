#ifndef LFR_AST_BLOCK_DECLS_H
#define LFR_AST_BLOCK_DECLS_H

struct ast_node;
struct inplocation_mark;

#include <analyzer/symbol_table.h>

struct ast_block {
    //! The block's value expression/statement. It can either be the last
    //! expression in the block or a return statement
    //! TODO: For now only one. There can be multiple returns.
    struct ast_node *value_expr;
    //! The block's symbol table. Only initialized in the analyzer phase
    struct symbol_table st;
};
#endif
