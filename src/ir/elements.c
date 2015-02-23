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
#include <String/rf_str_decl.h>
#include <String/rf_str_core.h>

/* -- rir_function -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_function, struct ast_node*, fn_impl, struct rir*, rir)
RF_STRUCT_INIT_SIG(rir_function, struct ast_node *fn_impl, struct rir *rir)
{
    AST_NODE_ASSERT_TYPE(fn_impl, AST_FUNCTION_IMPLEMENTATION);
    struct ast_node *fn_decl = ast_fnimpl_fndecl_get(fn_impl);
    const struct type *fn_type = ast_expression_get_type(fn_decl);

    // TODO: Somehow here create the parameters, so that names are also included.
    // We need them in the backend
    this->symbols = ast_fndecl_symbol_table_get(fn_decl);
    if (!rf_string_copy_in(&this->name, ast_fndecl_name_str(fn_decl))) {
        RF_ERROR("failed to iniailize rir_function name");
        return false;
    }

    this->arg_type = rir_type_create(type_function_get_argtype(fn_type), NULL, NULL);
    if (!this->arg_type) {
        RF_ERROR("Failed to create rir_function argument type");
        return false;
    }

    this->ret_type = rir_type_create(type_function_get_rettype(fn_type), NULL, NULL);
    if (!this->ret_type) {
        RF_ERROR("Failed to create rir_function return type");
        return false;
    }

    this->entry = rir_basic_blocks_create_from_ast_block(ast_fnimpl_body_get(fn_impl), rir);
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

/* -- rir_basic_block -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_basic_block)
RF_STRUCT_INIT_SIG(rir_basic_block)
{
    rf_ilist_head_init(&this->lh);
    return true;
}

RF_STRUCT_DEINIT_SIG(rir_basic_block)
{
    (void)this;
}

void rir_handle_block_expression(struct ast_node *n, struct rir_basic_block *b, struct rir *rir)
{
    struct ast_node *id;
    struct symbol_table_record *rec;
    switch(n->type) {
    case AST_VARIABLE_DECLARATION:
        // if it's a variable declaration get its rir type
        id = n->vardecl.desc->typedesc.left;
        AST_NODE_ASSERT_TYPE(id, AST_IDENTIFIER);
        rec = symbol_table_lookup_record(b->symbols,
                                         ast_identifier_str(id), NULL);
        if (!rec) {
            RF_ERROR("During RIR creation identifier was not found in block's symbol table");
            return;
        }
        rec->rir_data = rir_types_list_get_type(&rir->rir_types_list, rec->data, NULL);
        if (!rec->rir_data) {
            RF_ERROR("During RIR creation rir type corresponding to a normal type was not found");
            return;
        }
        break;
    case AST_BINARY_OPERATOR:
        rir_handle_block_expression(ast_binaryop_left(n), b, rir);
        rir_handle_block_expression(ast_binaryop_right(n), b, rir);
        rf_ilist_add_tail(&b->lh, &n->ln_for_rir_blocks);
        break;
    default:
        rf_ilist_add_tail(&b->lh, &n->ln_for_rir_blocks);
        break;
    }
}

struct rir_basic_block *rir_basic_blocks_create_from_ast_block(struct ast_node *n, struct rir *rir)
{
    struct ast_node *c;
    struct rir_basic_block *b;

    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    b = rir_basic_block_create();
    b->symbols = ast_block_symbol_table_get(n);
    rf_ilist_for_each(&n->children, c, lh) {
        // TODO depending on the children create other blocks and connect them to
        // this one but for now just simply ignore branching
        rir_handle_block_expression(c, b, rir);
    }

    return b;
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
