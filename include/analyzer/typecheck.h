#ifndef LFR_ANALYZER_TYPECHECK_H
#define LFR_ANALYZER_TYPECHECK_H

#include <Utils/sanity.h>
#include <Definitions/inline.h>

#include <stdbool.h>

struct symbol_table;
struct ast_node;
struct RFstring;

enum builtin_type {
    BUILTIN_UINT_8 = 0,
    BUILTIN_INT_8,
    BUILTIN_UINT_16,
    BUILTIN_INT_16,
    BUILTIN_UINT_32,
    BUILTIN_INT_32,
    BUILTIN_UINT_64,
    BUILTIN_INT_64,
    BUILTIN_FLOAT_32,
    BUILTIN_FLOAT_64,
    BUILTIN_STRING,

    BUILTIN_TYPES_COUNT /* keep as last */
};


enum type_category {
    TYPE_CATEGORY_ANONYMOUS = 0,
    TYPE_CATEGORY_BUILTIN,
    TYPE_CATEGORY_USER_DEFINED,
    TYPE_CATEGORY_FUNCTION,
};

struct type {
    enum type_category category;
    union {
        struct ast_node *decl;
        enum builtin_type btype;
    };
};

void type_builtin_init(struct type *t, enum builtin_type bt);
void type_init(struct type *t, enum type_category ctg, struct ast_node *decl);

i_INLINE_DECL enum builtin_type type_builtin(struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_BUILTIN,
              "Non built-in type category detected");
    return t->btype;
}


int analyzer_identifier_is_builtin(const struct RFstring *id);
bool analyzer_type_check(struct ast_node *type, struct symbol_table *st);

#endif
