#include <ir/elements.h>
#include <types/type.h>
#include <types/type_elementary.h>
#include <types/type_function.h>
#include <ast/ast.h>
#include <ast/type_decls.h>
#include <ast/block.h>
#include <ast/function.h>
#include <ast/vardecl.h>
#include <String/rf_str_decl.h>
#include <String/rf_str_core.h>

static struct rir_type i_elementary_types[] = {
#define INIT_ELEMENTARY_TYPE_ARRAY_INDEX(i_type)                           \
    [i_type] = {.elementary = i_type}

    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_8),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_16),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_INT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_UINT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_32),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_FLOAT_64),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_STRING),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_BOOL),
    INIT_ELEMENTARY_TYPE_ARRAY_INDEX(ELEMENTARY_TYPE_NIL)
#undef INIT_ELEMENTARY_TYPE_ARRAY_INDEX
};

static bool rir_type_init_iteration(struct rir_type *type, const struct type *input,
                                    const struct RFstring *name)
{
    // for now here we assume it's only product types
    struct rir_type *new_type;
    switch(input->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        if (type_elementary(input) != ELEMENTARY_TYPE_NIL) {
            /* RF_ASSERT(name, "For now, we always need to have a name in the rir type"); */
            new_type = rir_type_alloc(input);
            if (!rf_string_copy_in(&new_type->name, name)) {
                return false;
            }
            darray_push(type->subtypes, new_type);
        }
        break;
    case TYPE_CATEGORY_DEFINED:
        //TODO
        RF_ASSERT(false, "Not implemented yet");
        break;
    case TYPE_CATEGORY_OPERATOR:
        // TODO: make it work with sum types too
        if (input->operator.type != TYPEOP_PRODUCT) {
            RF_ASSERT(false, "Sum types not yet supported in the IR");
        }

        if (!rir_type_init_iteration(type, input->operator.left, NULL)) {
            return false;
        }
        if (!rir_type_init_iteration(type, input->operator.right, NULL)) {
            return false;
        }
        break;

    case TYPE_CATEGORY_LEAF:
        if (!rir_type_init_iteration(type, input->leaf.type, input->leaf.id)) {
            return false;
        }
        break;
    case TYPE_CATEGORY_GENERIC:
        RF_ASSERT(false, "Generic types not supported in the IR yet");
        break;
    }
    return true;
}

bool rir_type_init(struct rir_type *type, const struct type *input,
                   const struct RFstring *name)
{
    darray_init(type->subtypes);

    if (!rir_type_init_iteration(type, input, name)) {
        return false;
    }

    return true;
}

struct rir_type *rir_type_alloc(const struct type *input)
{
    if (input->category == TYPE_CATEGORY_ELEMENTARY) {
        return &i_elementary_types[type_elementary(input)];
    }

    // else
    struct rir_type *ret;
    RF_MALLOC(ret, sizeof(*ret), return NULL);
    return ret;

}

struct rir_type *rir_type_create(const struct type *input, const struct RFstring *name)
{
    struct rir_type *ret = rir_type_alloc(input);
    if (!ret) {
        RF_ERROR("Failed at rir_type allocation");
        return NULL;
    }
    if (!rir_type_init(ret, input, name)) {
        RF_ERROR("Failed at rir_type initialization");
        rir_type_dealloc(ret);
    }

    return ret;
}

void rir_type_dealloc(struct rir_type *t)
{
    // TODO: If t is not elementary then free
    (void)t;
}
void rir_type_destroy(struct rir_type *t)
{
    rf_string_deinit(&t->name);
    rir_type_deinit(t);
    rir_type_dealloc(t);
}
void rir_type_deinit(struct rir_type *t)
{
    struct rir_type **subtype;
    darray_foreach(subtype, t->subtypes) {
        rir_type_destroy(*subtype);
    }

    darray_free(t->subtypes);
}

const struct RFstring *rir_type_get_nth_name(struct rir_type *t, unsigned n)
{
    if (darray_size(t->subtypes) > n) {
        RF_ERROR("Requested rir_type name of subtype out of bounds");
        return NULL;
    }
    return &((struct rir_type*)darray_item(t->subtypes, n))->name;
}

const struct rir_type *rir_type_get_nth_type(struct rir_type *t, unsigned n)
{
    if (darray_size(t->subtypes) > n) {
        RF_ERROR("Requested rir_type type of subtype out of bounds");
        return NULL;
    }
    return darray_item(t->subtypes, n);
}

/* -- rir_function -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_function, struct ast_node*, fn_impl)
RF_STRUCT_INIT_SIG(rir_function, struct ast_node *fn_impl)
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

    this->entry = rir_basic_blocks_create_from_ast_block(ast_fnimpl_body_get(fn_impl));
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

struct rir_basic_block *rir_basic_blocks_create_from_ast_block(struct ast_node *n)
{
    struct ast_node *c;
    struct ast_node *id;
    struct rir_basic_block *b;
    struct rir_type *type;
    struct symbol_table_record *rec;

    AST_NODE_ASSERT_TYPE(n, AST_BLOCK);
    b = rir_basic_block_create();
    b->symbols = ast_block_symbol_table_get(n);
    rf_ilist_for_each(&n->children, c, lh) {
        // TODO depending on the children create other blocks and connect them to
        // this one but for now just simply ignore branching
        switch(c->type) {
        case AST_VARIABLE_DECLARATION:
            // if it's a variable declaration get/create its rir type
            type = rir_type_create(ast_expression_get_type(c), ast_vardecl_get_name(c));
            id = c->vardecl.desc->typedesc.left;
            AST_NODE_ASSERT_TYPE(id, AST_IDENTIFIER);
            rec = symbol_table_lookup_record(ast_block_symbol_table_get(n),
                                             ast_identifier_str(id), NULL);
            if (!rec) {
                RF_ERROR("During RIR creation identifier was not found in block's symbol table");
                return NULL;
            }
            rec->rir_data = type;

        default:
            rf_ilist_add_tail(&b->lh, &c->ln_for_rir_blocks);
            break;
        }
    }

    return b;
}

/* -- rir_module -- */
RF_STRUCT_COMMON_DEFS_NO_ALLOC(rir_module, struct ast_node*, n, const struct RFstring*, name)
RF_STRUCT_INIT_SIG(rir_module, struct ast_node *n, const struct RFstring *name)
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
        // for now any non function children trigger failure
        if (c->type != AST_FUNCTION_IMPLEMENTATION) {
            goto fail;
        }

        fn = rir_function_create(c);
        if (!fn) {
            goto fail;
        }
        rf_ilist_add_tail(&this->functions, &fn->ln_for_module);
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
