#ifndef LFR_AST_FUNCTIONS_H
#define LFR_AST_FUNCTIONS_H

#include <rfbase/utils/sanity.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/ast_utils.h>
#include <types/type_decls.h>
#include <analyzer/symbol_table.h>

struct ast_argument;

/* -- function declaration functions -- */

struct ast_node *ast_fndecl_create(
    const struct inplocation_mark *start,
    const struct inplocation_mark *end,
    enum fndecl_position pos,
    struct ast_node *name,
    struct ast_node *genr,
    struct ast_node *args,
    struct ast_node *ret
);

void ast_fndecl_deinit(struct ast_node *n);


i_INLINE_DECL const struct RFstring *ast_fndecl_name_str(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return ast_identifier_str(n->fndecl.name);
}

i_INLINE_DECL bool ast_fndecl_symbol_table_init(struct ast_node *n,
                                                struct module *m)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return symbol_table_init(&n->fndecl.st, m);
}

i_INLINE_DECL struct symbol_table *ast_fndecl_symbol_table_get(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return &n->fndecl.st;
}

i_INLINE_DECL struct ast_node *ast_fndecl_genrdecl_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return n->fndecl.genr;
}

i_INLINE_DECL struct ast_node *ast_fndecl_args_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return n->fndecl.args;
}

i_INLINE_DECL struct ast_node *ast_fndecl_return_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return n->fndecl.ret;
}

i_INLINE_DECL enum fndecl_position ast_fndecl_position_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    return n->fndecl.position;
}

i_INLINE_DECL unsigned ast_fndecl_argsnum_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_DECLARATION);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fndecl.args_num;
}

/**
 * Returns a specific argument from the function or NULL if out of bounds.
 *
 * @param n        The function declaration for which to search
 * @param idx      The index of the argument
 * @return         The argument at idx if found, or NULL otherwise.
 */
struct ast_argument *ast_fndecl_argument_get(const struct ast_node *n, unsigned idx);

/**
 * Returns if the first argument of the function is `self`
 */
bool ast_fndecl_firstarg_is_self(const struct ast_node *n);


/* -- function implementation functions -- */

struct ast_node *ast_fnimpl_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *decl,
                                   struct ast_node *body);

i_INLINE_DECL struct ast_node *ast_fnimpl_fndecl_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    return n->fnimpl.decl;
}

i_INLINE_DECL struct ast_node *ast_fnimpl_body_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    return n->fnimpl.body;
}

i_INLINE_DECL void ast_fnimpl_symbol_table_set(struct ast_node *n,
                                               struct symbol_table *st)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    n->fnimpl.st = st;
}

i_INLINE_DECL struct symbol_table *ast_fnimpl_symbol_table_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    return n->fnimpl.st;
}

i_INLINE_DECL const struct RFstring *ast_fnimpl_namestr_get(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    return ast_fndecl_name_str(n->fnimpl.decl);
}

/**
 * Returns if the first argument of the function is `self`
 */
i_INLINE_DECL bool ast_fnimpl_firstarg_is_self(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_IMPLEMENTATION);
    return ast_fndecl_firstarg_is_self(n->fnimpl.decl);
}

/* -- function call functions -- */

struct ast_node *ast_fncall_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   struct ast_node *name,
                                   struct ast_node *args,
                                   struct ast_node *genr);

i_INLINE_DECL const struct RFstring* ast_fncall_name(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    return ast_identifier_str(n->fncall.name);
}

i_INLINE_DECL struct ast_node* ast_fncall_args(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    return n->fncall.args;
}

i_INLINE_DECL struct ast_node* ast_fncall_genr(struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    return n->fncall.genr;
}

/**
 * @return the type of the parameters this function call has.
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL const struct type *ast_fncall_params_type(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fncall.params_type;
}

/**
 * @return if this is a call to a function that has sum type arguments
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_fncall_is_sum(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fncall.type == AST_FNCALL_SUM;
}

/**
 * @return the original type of the function declaration this call refers to
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL const struct type *ast_fncall_type(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fncall.declared_type;
}

/**
 * @return true if this function call is constructor call
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_fncall_is_ctor(const struct ast_node *n)
{
    const struct type *t = ast_fncall_type(n);
    return t ? t->category == TYPE_CATEGORY_DEFINED : false;
}

/**
 * @return true if this ast_node is a constructor call
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_node_is_ctor(const struct ast_node *n)
{
    return n->type == AST_FUNCTION_CALL ? ast_fncall_is_ctor(n) : false;
}

/**
 * @return true if this function call is a conversion call
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_fncall_is_conversion(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fncall.type == AST_FNCALL_EXPLICIT_CONVERSION;
}

/**
 * @return true if this is a call to a foreign function
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_fncall_is_foreign(const struct ast_node *n)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    AST_NODE_ASSERT_STATE(n, AST_NODE_STATE_RIR_END);
    return n->fncall.type == AST_FNCALL_FOREIGN;
}

/**
 * @return true if this ast_node is a conversion call
 * @warning: Valid only after typechecking.
 */
i_INLINE_DECL bool ast_node_is_conversion(const struct ast_node *n)
{
    return n->type == AST_FUNCTION_CALL ? ast_fncall_is_conversion(n) : false;
}

/**
 * @return true if this ast_node is a function call type that already had its
 * return value processed/assigned during its creation.
 */
i_INLINE_DECL bool ast_node_is_fncall_preprocessed(const struct ast_node *n)
{
    /* return n->type == AST_FUNCTION_CALL && (ast_fncall_is_ctor(n) || !ast_fncall_is_conversion(n)); */
    return n->type == AST_FUNCTION_CALL && (ast_fncall_is_ctor(n));
}

/**
 * Get or create the arguments of a function call.
 *
 * The first time this function is called the arguments array is created
 * and then all subsequent calls simply return it
 *
 * @param n    The ast function call whose arguments to get/create
 * @return     the arguments array of the function call
 */
struct arr_ast_nodes *ast_fncall_arguments(struct ast_node *n);

/**
 * A way to iterate a function call's arguments before typechecking
 * creates the array of ast_node members
 *
 * @param n        The function call whose ast node arguments to iterate
 * @param cb       The callback function to call for each call argument
 * @param user     The extra user argument to provide to the callback
 */
i_INLINE_DECL bool ast_fncall_foreach_arg(
    const struct ast_node *n,
    exprlist_cb cb,
    void *user
)
{
    AST_NODE_ASSERT_TYPE(n, AST_FUNCTION_CALL);
    RF_ASSERT(
        n->state < AST_NODE_STATE_TYPECHECK_1,
        "Do not use this function after typechecking"
    );
    struct ast_node *args = ast_fncall_args(n);
    if (!args) {
        return true;
    }
    return ast_foreach_expr(args, cb, user);
}
#endif
