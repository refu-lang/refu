#ifndef LFR_AST_TYPECLASS_DECLS_H
#define LFR_AST_TYPECLASS_DECLS_H

struct ast_typeclass
{
    struct ast_node *name;
    struct ast_node *generics;
};

struct ast_typeinstance
{
    struct ast_node *class_name;
    struct ast_node *type_name;
    struct ast_node *generics;
};

#endif
