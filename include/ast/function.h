#ifndef LFR_AST_FUNCTIONS_H
#define LFR_AST_FUNCTIONS_H

#include <ast/ast.h>
#include <Utils/sanity.h>
#include <ast/identifier.h>
#include <types/type_decls.h>

#include <analyzer/symbol_table.h>

/* -- function declaration functions -- */

struct ast_node *ast_fndecl_create(const struct inplocation_mark *start,
                                   const struct inplocation_mark *end,
                                   enum fndecl_position pos,
                                   struct ast_node *name,
                                   struct ast_node *genr,
                                   struct ast_node *args,
                                   struct ast_node *ret);


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

struct RFstring *ast_fndecl_ret_str(struct ast_node *n);

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
    return n->fncall.sumcall;
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
    return n->fncall.is_explicit_conversion;
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

typedef bool (*fncall_args_cb) (struct ast_node *n, void *user_arg);
/**
 * Function call arguments iteration callback.
 *
 * Should be called only after typechecking
 *
 * @param n            The function call ast node
 * @param cb           The callback to execute for each argument expression
 * @param user_arg     The extra argument to provide to the callback
 */
bool ast_fncall_for_each_arg(const struct ast_node *n, fncall_args_cb cb, void *user_arg);

#endif
