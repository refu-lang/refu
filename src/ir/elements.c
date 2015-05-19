#include <ir/elements.h>
#include <ir/rir.h>
#include <ir/rir_type.h>
#include <ir/rir_types_list.h>
#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>
#include <ast/ast.h>
#include <ast/type_decls.h>
#include <ast/block.h>
#include <ast/function.h>
#include <ast/vardecl.h>
#include <ast/operators.h>
#include <ast/ifexpr.h>
#include <String/rf_str_decl.h>
#include <String/rf_str_core.h>

/* -- rir_function -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_function, struct ast_node*, fn_impl, struct rir*, rir)
RF_STRUCT_INIT_SIG(rir_function, struct ast_node *fn_impl, struct rir *rir)
{
    AST_NODE_ASSERT_TYPE(fn_impl, AST_FUNCTION_IMPLEMENTATION);
    struct ast_node *fn_decl = ast_fnimpl_fndecl_get(fn_impl);
    const struct type *fn_type = ast_node_get_type(fn_decl);

    // TODO: Somehow here create the parameters, so that names are also included.
    // We need them in the backend
    this->symbols = ast_fndecl_symbol_table_get(fn_decl);
    if (!rf_string_copy_in(&this->name, ast_fndecl_name_str(fn_decl))) {
        RF_ERROR("failed to iniailize rir_function name");
        return false;
    }

    this->arg_type = rir_type_create(type_function_get_argtype(fn_type), NULL);
    if (!this->arg_type) {
        RF_ERROR("Failed to create rir_function argument type");
        return false;
    }

    this->ret_type = rir_type_create(type_function_get_rettype(fn_type), NULL);
    if (!this->ret_type) {
        RF_ERROR("Failed to create rir_function return type");
        return false;
    }

    this->entry = rir_basic_blocks_create_from_ast_block(
        ast_fnimpl_body_get(fn_impl),
        ast_fnimpl_symbol_table_get(fn_impl),
        rir);
    if (!this->entry) {
        RF_ERROR("Failed to create rir_basic_block for a function");
        return false;
    }

    return true;
}

RF_STRUCT_DEINIT_SIG(rir_function)
{
    rf_string_deinit(&this->name);
    rir_type_destroy(this->arg_type);
    rir_type_destroy(this->ret_type);
    rir_basic_block_destroy(this->entry);
}

/* -- rir_branch -- */
bool rir_cond_branch_init(struct rir_cond_branch *rir_branch,
                          struct ast_node *n,
                          struct symbol_table *st,
                          struct rir *rir)
{
    struct ast_node *fallthrough;

    rir_branch->cond = ast_ifexpr_condition_get(n);
    rir_branch->true_br = rir_basic_blocks_create_from_ast_block(
        ast_ifexpr_taken_block_get(n), st, rir
    );

    rir_branch->false_br = NULL;
    fallthrough = ast_ifexpr_fallthrough_branch_get(n);
    if (fallthrough) {
        AST_NODE_ASSERT_TYPE(fallthrough, AST_CONDITIONAL_BRANCH || AST_BLOCK);
        rir_branch->false_br = rir_branch_create(fallthrough, st, rir);
        if (!rir_branch->false_br) {
            RF_ERROR("Failed to create a RIR fallthrough branch");
            return false;
        }
    }

    return true;
}

struct rir_cond_branch *rir_cond_branch_create(struct ast_node *n,
                                               struct symbol_table *st,
                                               struct rir *r)
{
    struct rir_cond_branch *ret;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    if (!rir_cond_branch_init(ret, n, st, r)) {
        return NULL;
    }
    return ret;
}

void rir_cond_branch_deinit(struct rir_cond_branch *rir_cond)
{
    rir_basic_block_destroy(rir_cond->true_br);
    if (rir_cond->false_br) {
        rir_branch_destroy(rir_cond->false_br);
    }
}

struct rir_branch *rir_branch_create(struct ast_node *node,
                                     struct symbol_table *st,
                                     struct rir *rir)
{
    struct rir_branch *ret;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    ret->is_conditional = false;

    AST_NODE_ASSERT_TYPE(node, AST_IF_EXPRESSION || AST_BLOCK);
    if (node->type == AST_IF_EXPRESSION) {
        ret->is_conditional = true;
        rir_cond_branch_init(&ret->cond_branch, node, st, rir);
    } else {
        ret->simple_branch = rir_basic_blocks_create_from_ast_block(node, st, rir);
    }

    return ret;
}

void rir_branch_destroy(struct rir_branch *branch)
{
    if (branch->is_conditional) {
        rir_cond_branch_deinit(&branch->cond_branch);
    } else {
        rir_basic_block_destroy(branch->simple_branch);
    }
    free(branch);
}

/* -- rir_basic_block -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_basic_block)
RF_STRUCT_INIT_SIG(rir_basic_block)
{
    rf_ilist_head_init(&this->expressions);
    return true;
}

RF_STRUCT_DEINIT_SIG(rir_basic_block)
{
    struct rir_expression *expr;
    struct rir_expression *tmp;
    rf_ilist_for_each_safe(&this->expressions, expr, tmp, ln) {
        rir_expression_destroy(expr);
    }
}

bool rir_handle_block_expression(struct ast_node *n, struct rir_basic_block *b, struct rir *rir)
{
    struct ast_node *id;
    struct symbol_table_record *rec;
    switch(n->type) {
    case AST_VARIABLE_DECLARATION:
        // if it's a variable declaration get its rir type
        id = ast_typeleaf_left(ast_vardecl_desc_get(n));
        AST_NODE_ASSERT_TYPE(id, AST_IDENTIFIER);
        rec = symbol_table_lookup_record(b->symbols,
                                         ast_identifier_str(id), NULL);

        RF_ASSERT(rec, "During RIR creation identifier was not found in block's symbol table");
        rec->rir_data = rir_types_list_get_type(&rir->rir_types_list, rec->data, NULL);
        RF_ASSERT(rec->rir_data, "During RIR creation rir type corresponding to a normal type was not found");

        break;
    case AST_IF_EXPRESSION:
        return rir_expression_create(b, n, RIR_IF_EXPRESSION, rir);
    default:
        return rir_expression_create(b, n, RIR_SIMPLE_EXPRESSION, rir);
    }

    return true;
}

struct rir_basic_block *rir_basic_blocks_create_from_ast_block(
    struct ast_node *n,
    struct symbol_table *st,
    struct rir *rir)
{
    struct ast_node *c;
    struct rir_basic_block *b;

    b = rir_basic_block_create();
    if (n->type == AST_BLOCK) { // ugly as hell. Go away with RIR refactor.
        b->symbols = ast_block_symbol_table_get(n);
        b->normal_block = true;
        rf_ilist_for_each(&n->children, c, lh) {
            // TODO depending on the children create other blocks and connect them to
            // this one but for now just simply ignore branching
            if (!rir_handle_block_expression(c, b, rir)) {
                return NULL;
            }
        }
    } else {
        b->symbols = st;
        b->normal_block = false;
        if (!rir_expression_create(b, n, RIR_SIMPLE_EXPRESSION, rir)) {
            return NULL;
        }
    }


    return b;
}

/* -- rir_expression -- */
bool rir_expression_init(struct rir_expression *expr,
                         struct rir_basic_block *parent,
                         struct ast_node *node,
                         enum rir_expression_type type,
                         struct rir *rir)
{
    expr->type = type;

    switch(expr->type) {
    case RIR_SIMPLE_EXPRESSION:
        expr->expr = node;
        break;
    case RIR_IF_EXPRESSION:
        expr->branch = rir_branch_create(node, parent->symbols, rir);
        if (!expr->branch) {
            return false;
        }
        break;
    }

    rf_ilist_add_tail(&parent->expressions, &expr->ln);
    return true;
}

struct rir_expression *rir_expression_create(struct rir_basic_block *parent,
                                             struct ast_node *node,
                                             enum rir_expression_type type,
                                             struct rir *rir)
{
    struct rir_expression *ret;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    if (!rir_expression_init(ret, parent, node, type, rir)) {
        return NULL;
    }
    return ret;
}

void rir_expression_deinit(struct rir_expression *expr)
{
    if (expr->type == RIR_IF_EXPRESSION) {
        rir_branch_destroy(expr->branch);
    }
}
void rir_expression_destroy(struct rir_expression *expr)
{
    rir_expression_deinit(expr);
    free(expr);
}

/* -- rir_module -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_module, struct ast_node*, n,
                               const struct RFstring*, name,
                               struct rir *, rir)
RF_STRUCT_INIT_SIG(rir_module, struct ast_node *n, const struct RFstring *name, struct rir *rir)
{
    struct ast_node *c;
    struct rir_function *fn;
    struct rir_function *tmp;
    AST_NODE_ASSERT_TYPE(n, AST_ROOT);

    if (!rf_string_copy_in(&this->name, name)) {
        RF_ERROR("Failed to copy rir_module name");
        return false;
    }

    this->symbols = &n->root.st;
    rf_ilist_head_init(&this->functions);
    rf_ilist_for_each(&n->children, c, lh) {
        switch (c->type) {
        case AST_FUNCTION_IMPLEMENTATION:
            fn = rir_function_create(c, rir);
            if (!fn) {
                goto fail;
            }
            rf_ilist_add_tail(&this->functions, &fn->ln_for_module);
            break;
        case AST_TYPE_DECLARATION:
            // all types should be in the global rir types list
            break;
        default:
            RF_ASSERT(false, "Unexpected ast node \""RF_STR_PF_FMT"\" at the"
                      " top level of a module", RF_STR_PF_ARG(ast_node_str(c)));
            goto fail;
            break;
        }
    }

    return true;

fail:
    rf_ilist_for_each_safe(&this->functions, fn, tmp, ln_for_module) {
        rir_function_destroy(fn);
    }
    return false;
}

RF_STRUCT_DEINIT_SIG(rir_module)
{
    struct rir_function *fn;
    struct rir_function *tmp;
    rf_string_deinit(&this->name);
    rf_ilist_for_each_safe(&this->functions, fn, tmp, ln_for_module) {
        rir_function_destroy(fn);
    }
}
