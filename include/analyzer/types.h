#ifndef LFR_ANALYZER_TYPES_H
#define LFR_ANALYZER_TYPES_H

#include <stdbool.h>

#include <Utils/sanity.h>
#include <Definitions/inline.h>
#include <String/rf_str_decl.h>
#include <Data_Structures/intrusive_list.h>

#include <ast/type_decls.h>

struct analyzer;
struct symbol_table;

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
    TYPE_CATEGORY_ANONYMOUS = 0,        /* an anonymous type */
    TYPE_CATEGORY_LEAF,                 /* almost always part of another type */
    TYPE_CATEGORY_BUILTIN,              /* a builtin type */
    TYPE_CATEGORY_USER_DEFINED,         /* a user defined type */
    TYPE_CATEGORY_GENERIC,              /* a generic type as declared by the user */
    TYPE_CATEGORY_FUNCTION,             /* a function */
};

struct type_builtin {
    enum builtin_type btype;
};

struct type_generic {
    const struct RFstring id;
};

struct type_leaf {
    const struct RFstring *id;
    struct type *type;
};

struct type_operator {
    enum typeop_type type;
    struct type *left;
    struct type *right;
};

struct type_composite {
    bool is_operator;
    union {
        struct type_operator op;
        struct type_leaf leaf;
    };
};

struct type_defined {
    const struct RFstring *id;
    struct type_composite *type;
};

struct type_function {
    struct type *argument_type;
    struct type *return_type;
};

struct type {
    enum type_category category;
    /* list handler, to be added to either types or anonymous types list */
    struct RFilist_node lh;
    union {
        struct type_builtin builtin;
        struct type_defined defined;
        struct type_function function;
        struct type_composite anonymous;
        struct type_leaf leaf;
    };
};

/* -- type allocation functions -- */

struct type *type_alloc(struct analyzer *a);
void type_free(struct type *t, struct analyzer *a);

/* -- various type creation and initialization functions -- */

struct type *type_create(struct ast_node *n, struct analyzer *a,
                         struct symbol_table *st,
                         struct ast_node *genrdecl);

struct type *type_create_from_typedecl(struct ast_node *n,
                                       struct analyzer *a,
                                       struct symbol_table *st);

struct type *type_create_from_vardecl(struct ast_node *n,
                                      struct analyzer *a,
                                      struct symbol_table *st);

struct type *type_create_from_fndecl(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st);

struct type *type_anonymous_create(struct ast_node *n,
                                   struct analyzer *a,
                                   struct symbol_table *st,
                                   struct ast_node *genrdecl);

/* -- type comparison functions -- */

bool type_equals(struct type* t1, struct type *t2);
bool type_equals_typedesc(struct type *t, struct ast_node *type_desc,
                          struct analyzer *a, struct symbol_table *st,
                          struct ast_node *genrdecl);

struct type *type_lookup_xidentifier(struct ast_node *n,
                                     struct analyzer *a,
                                     struct symbol_table *st,
                                     struct ast_node *genrdecl);

/* -- type getters -- */

i_INLINE_DECL enum builtin_type type_builtin(struct type *t)
{
    RF_ASSERT(t->category == TYPE_CATEGORY_BUILTIN,
              "Non built-in type category detected");
    return t->builtin.btype;
}

/* -- type traversal functions -- */

typedef bool (*leaf_type_cb) (struct type_leaf *t, void *user_arg);

bool type_for_each_leaf(struct type *t, leaf_type_cb cb, void *user_arg);


#endif
