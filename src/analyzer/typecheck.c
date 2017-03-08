#include <analyzer/typecheck.h>

#include <rfbase/utils/build_assert.h>
#include <rfbase/utils/bits.h>
#include <rfbase/persistent/buffers.h>
#include <rfbase/string/core.h>
#include <rfbase/string/conversion.h>

#include <module.h>
#include <ast/ast.h>
#include <ast/arr.h>
#include <ast/operators.h>
#include <ast/function.h>
#include <ast/forexpr.h>
#include <ast/block.h>
#include <ast/constants.h>
#include <ast/vardecl.h>
#include <ast/ast_utils.h>
#include <ast/returnstmt.h>
#include <ast/type.h>
#include <ast/module.h>
#include <ast/typeclass.h>

#include <types/type.h>
#include <types/type_arr.h>
#include <types/type_operators.h>
#include <types/type_comparisons.h>
#include <types/type_elementary.h>
#include <types/type_function.h>

#include <analyzer/analyzer.h>
#include <analyzer/symbol_table.h>
#include <analyzer/typecheck_forexpr.h>
#include <analyzer/typecheck_matchexpr.h>
#include <analyzer/typecheck_arr.h>
#include <analyzer/typecheck_functions.h>
#include <analyzer/typecheck_typeclass.h>
#include <analyzer/analyzer_pass1.h> // for analyzer symbol table change functions

void traversal_node_set_type(
    struct ast_node *n,
    const struct type *t,
    struct analyzer_traversal_ctx *ctx)
{
    n->expression_type = t;
    ctx->last_node_type = t;
}

static bool typecheck_binaryop_get_operands(
    struct ast_node *n,
    struct ast_node *left,
    const struct type **tleft,
    struct ast_node *right,
    const struct type **tright,
    struct analyzer_traversal_ctx *ctx)
{
    *tleft = ast_node_get_type(left);
    if (!*tleft) {
        analyzer_err(ctx->m, ast_node_startmark(left), ast_node_endmark(left),
                     "Type of left side of \""RFS_PF"\" can not be determined",
                     RFS_PA(ast_binaryop_opstr(n)));
        return false;
    }
    *tright = ast_node_get_type(right);
    if (!*tright) {
        analyzer_err(ctx->m, ast_node_startmark(right), ast_node_endmark(right),
                     "Type of right side of \""RFS_PF"\" can not be determined",
                     RFS_PA(ast_binaryop_opstr(n)));
        return false;
    }
    return true;
}

static bool i_should_be_changed(
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    // TODO: this is just a callback which should never exist. Should check if op is applicable
    // Wherever it's used it's just temporary and should be changed to
    // either a callback per binary operator or something else
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static const struct type *typecheck_do_type_conversion(
    const struct type *left,
    const struct type *right,
    struct analyzer_traversal_ctx *ctx)
{
    // if both are elementary always try to convert the smaller to the bigger size
    if (left->category == TYPE_CATEGORY_ELEMENTARY &&
        right->category == TYPE_CATEGORY_ELEMENTARY) {
        if (right->elementary.etype >= left->elementary.etype &&
            type_compare(left, right, TYPECMP_IMPLICIT_CONVERSION)) {
            // return right but without constant type
            return
                type_is_constant_elementary(right) ?
                type_elementary_get_type(right->elementary.etype) : right;
        } else if (type_compare(right, left, TYPECMP_IMPLICIT_CONVERSION)) {
            // return left but without constant type
            return
                type_is_constant_elementary(left) ?
                type_elementary_get_type(left->elementary.etype) : left;
        }
    //  else try to see if either side can be implicitly converted to another
    } else if (type_compare(left, right, TYPECMP_IMPLICIT_CONVERSION)) {
        return right;
    } else if (type_compare(right, left, TYPECMP_IMPLICIT_CONVERSION)) {
        return left;
    }

    return NULL;
}

static enum traversal_cb_res typecheck_unaryop_generic(
    struct ast_node *n,
    const struct type *operand_type,
    const char *error_intro,
    const char *error_conj,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *final_type = NULL;

    // for now only applicable to numeric elementary types
    if (!type_is_numeric_elementary(operand_type)) {
        goto fail;
    }
    final_type = operand_type;
    traversal_node_set_type(n, final_type, ctx);
    return TRAVERSAL_CB_OK;

fail:
    n->expression_type = NULL;
    RFS_PUSH();
    analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                 "%s \""RFS_PF"\" %s \""RFS_PF"\"",
                 error_intro, RFS_PA(ast_unaryop_opstr(n)),
                 error_conj, RFS_PA(type_str_or_die(operand_type, TSTR_DEFAULT)));
    RFS_POP();
    return TRAVERSAL_CB_ERROR;
}

// Generic typecheck function for binary operations. If special functionality
// needs to be implemented for an operator do that in a separate function
static enum traversal_cb_res typecheck_binaryop_generic(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx,
    enum binaryop_type operation,
    bool(*operator_applicable_cb)(struct ast_node*, struct ast_node*, struct analyzer_traversal_ctx*),
    const char *error_intro,
    const char *error_conj)
{
    enum traversal_cb_res rc = TRAVERSAL_CB_ERROR;
    const struct type *tright;
    const struct type *tleft;
    const struct type *final_type = NULL;

    if (!typecheck_binaryop_get_operands(n, left, &tleft, right, &tright, ctx)) {
        return TRAVERSAL_CB_ERROR;
    }

    final_type = typecheck_do_type_conversion(tleft, tright, ctx);
    RFS_PUSH();
    if (!final_type && !operator_applicable_cb(left, right, ctx)) {
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "%s \""RFS_PF"\" %s \""RFS_PF"\"",
                     error_intro, RFS_PA(type_str_or_die(tright, TSTR_DEFAULT)),
                     error_conj, RFS_PA(type_str_or_die(tleft, TSTR_DEFAULT)));
        goto end;
    }

    n->binaryop.common_type = final_type;
    traversal_node_set_type(n, final_type, ctx);
    rc = TRAVERSAL_CB_OK;

    end:
    RFS_POP();
    return rc;
}

// Generic typecheck function for binary operations whose type should be a boolean
static enum traversal_cb_res typecheck_bool_binaryop_generic(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx,
    enum binaryop_type operation,
    bool(*operator_applicable_cb)(struct ast_node*, struct ast_node*, struct analyzer_traversal_ctx*),
    const char *error_intro,
    const char *error_conj)
{
    const struct type *tright;
    const struct type *tleft;
    const struct type *final_type = NULL;

    if (!typecheck_binaryop_get_operands(n, left, &tleft, right, &tright, ctx)) {
        return TRAVERSAL_CB_ERROR;
    }

    final_type = typecheck_do_type_conversion(tleft, tright, ctx);
    if (!final_type && !operator_applicable_cb(left, right, ctx)) {
        RFS_PUSH();
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "%s \""RFS_PF"\" %s \""RFS_PF"\"",
                     error_intro,
                     RFS_PA(type_str_or_die(tright, TSTR_DEFAULT)),
                     error_conj,
                     RFS_PA(type_str_or_die(tleft, TSTR_DEFAULT)));
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }

    n->binaryop.common_type = final_type;
    traversal_node_set_type(n, type_elementary_get_type(ELEMENTARY_TYPE_BOOL), ctx);
    return TRAVERSAL_CB_OK;
}



struct typecheck_member_access_iter_ctx {
    struct ast_node *member_identifier;
    struct type *member_type;
};

static inline void typecheck_member_access_iter_ctx_init(
    struct typecheck_member_access_iter_ctx *ctx,
    struct ast_node *member_identifier)
{
    ctx->member_identifier = member_identifier;
    ctx->member_type = NULL;
}

static bool typecheck_member_access_iter_cb(
    const struct RFstring *name,
    const struct ast_node *desc,
    struct type *t,
    struct typecheck_member_access_iter_ctx *ctx)
{
    (void)desc;
    if (rf_string_equal(name, ast_identifier_str(ctx->member_identifier))) {
        ctx->member_type = t;
        ctx->member_identifier->expression_type = t;
        return false; //stop the iteration from the callback
    }
    return true;
}

static enum traversal_cb_res typecheck_member_access(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *tleft;
    struct typecheck_member_access_iter_ctx member_access_iter_ctx;

    tleft = ast_node_get_type(left);
    if (!tleft) {
        analyzer_err(ctx->m, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Undeclared identifier \""RFS_PF"\" as left part of "
                     "member access operator",
                     RFS_PA(ast_identifier_str(left)));
        return TRAVERSAL_CB_ERROR;
    }

    // left type of member access should be a custom defined type
    if (!type_category_equals(tleft, TYPE_CATEGORY_DEFINED)) {
        analyzer_err(ctx->m, ast_node_startmark(left),
                     ast_node_endmark(left),
                     "Left part of member access operator \""RFS_PF"\" is "
                     "not a user defined type",
                     RFS_PA(ast_node_str(left)));
        return TRAVERSAL_CB_ERROR;
    }

    // right type of member access should be an identifier (at least for now)
    if (right->type != AST_IDENTIFIER) {
        analyzer_err(ctx->m, ast_node_startmark(right),
                     ast_node_endmark(right),
                     "Right part of member access operator is not an identifier"
                     " but is \""RFS_PF"\"",
                     RFS_PA(ast_node_str(right)));
        return TRAVERSAL_CB_ERROR;
    }

    // get ast type description of the left member
    const struct ast_node *desc = symbol_table_lookup_node(
        ctx->current_st,
        ast_identifier_str(left),
        NULL
    );
    if (!desc) {
        // should not happen
        RF_ERROR("Could not retrieve ast description from the symbol table");
        return TRAVERSAL_CB_ERROR;
    }
    if (desc->type == AST_TYPE_LEAF) {
        // if it's a typeleaf we need the type description of it's right member
        desc = ast_typeleaf_right(desc);
        if (desc->type == AST_XIDENTIFIER) {
            // if it's an identifier we need to look it up again
            desc = symbol_table_lookup_node(ctx->current_st, ast_identifier_str(desc), NULL);
        }
    }
    typecheck_member_access_iter_ctx_init(&member_access_iter_ctx, right);
    ast_type_foreach_leaf_arg(
        desc,
        tleft->defined.type,
        (ast_type_cb)typecheck_member_access_iter_cb,
        &member_access_iter_ctx
    );
    if (!member_access_iter_ctx.member_type) {
        RFS_PUSH();
        analyzer_err(
            ctx->m, ast_node_startmark(n),
            ast_node_endmark(n),
            "Could not find member \""RFS_PF"\" in type \""RFS_PF"\"",
            RFS_PA(ast_identifier_str(right)),
            RFS_PA(type_str_or_die(tleft, TSTR_DEFAULT))
        );
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }

    traversal_node_set_type(n, member_access_iter_ctx.member_type, ctx);
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_constant(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    traversal_node_set_type(n, ast_constant_get_storagetype(n), ctx);
    return ast_node_get_type(n) ? TRAVERSAL_CB_OK : TRAVERSAL_CB_ERROR;
}

// figure out if any of the wildcard's close parents is a matchcase
static bool wilcard_parent_is_matchcase(const struct ast_node *n, void *user_arg)
{
    return n->type == AST_MATCH_CASE;
}

static enum traversal_cb_res typecheck_identifier(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    AST_NODE_ASSERT_TYPE(n, AST_IDENTIFIER);
    if (ast_identifier_is_wildcard(n)) {
        if (!analyzer_traversal_ctx_traverse_parents(
                ctx,
                wilcard_parent_is_matchcase,
                NULL
            )) {

            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Reserved wildcard identifier '_' used outside of "
                " a match expression"
            );
            return TRAVERSAL_CB_ERROR;
        }
        traversal_node_set_type(n, type_get_wildcard(), ctx);
        return TRAVERSAL_CB_OK;
    } else if (ast_identifier_is_self(n)) {
        const struct type *typeinstance_type = symbol_table_check_and_get_selftype(ctx->current_st);
        if (!typeinstance_type) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Reserved identifier 'self' used outside of a typeclass"
            );
            return TRAVERSAL_CB_ERROR;
        }

        traversal_node_set_type(n, typeinstance_type, ctx);
        return TRAVERSAL_CB_OK;
    }

    traversal_node_set_type(
        n,
        type_lookup_identifier_string(ast_identifier_str(n), ctx->current_st),
        ctx
    );

    const struct type *id_type = ast_node_get_type(n);
    if (id_type) {
        return TRAVERSAL_CB_OK;
    }
    // for some identifiers, like for the right part of a member access it's
    // impossible to determine type at this stage, for the rest it's an error
    struct ast_node *parent = analyzer_traversal_ctx_get_nth_parent_or_die(0, ctx);
    struct ast_node *parent_2 = analyzer_traversal_ctx_get_nth_parent(1, ctx);
    bool parent_member_access = (
        ast_node_is_specific_binaryop(parent, BINARYOP_MEMBER_ACCESS) ||
        (parent_2 && ast_node_is_specific_binaryop(parent, BINARYOP_MEMBER_ACCESS))
    );
    if (parent_member_access ||  parent->type == AST_IMPORT) {
        return TRAVERSAL_CB_OK;
    }
    analyzer_err(
        ctx->m,
        ast_node_startmark(n),
        ast_node_endmark(n),
        "Undeclared identifier \""RFS_PF"\"",
        RFS_PA(ast_identifier_str(n))
    );
    return TRAVERSAL_CB_ERROR;
}

static enum traversal_cb_res typecheck_xidentifier(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    traversal_node_set_type(
        n,
        ast_node_get_type(n->xidentifier.id),
        ctx
    );
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_typeleaf(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // an ast_type_leaf's type is a type leaf
    type_creation_ctx_set_args(ctx->m, ctx->current_st, NULL);
    traversal_node_set_type(n, module_get_or_create_type(n), ctx);
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_typedesc(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // a type descriptions's type is the type of its description
    traversal_node_set_type(
        n,
        ast_node_get_type(ast_typedesc_desc_get(n)),
        ctx
    );
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_typedecl(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    traversal_node_set_type(
        n,
        ast_node_get_type(ast_typedecl_typedesc_get(n)),
        ctx
    );
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_typeop(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // during symbol table population, if the typeoperator is a child of some specific
    // nodes there will be calls to analyzer_get_or_create_type() and the type will
    // already exists. If so just return
    if (ast_node_get_type(n)) {
        return TRAVERSAL_CB_OK;
    }

    // for the rest we need to create it here
    type_creation_ctx_set_args(ctx->m, ctx->current_st, NULL);
    n->expression_type = type_lookup_or_create(n);
    RF_ASSERT_OR_EXIT(n->expression_type, "Could not determine type of matchase type operation");
    return TRAVERSAL_CB_OK;
}

static bool analyzer_types_assignable(
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    // TODO: Check if two types that are not strictly equal can still be assignable
    // if for example there is an assignment typeclass instance
    (void)left;
    (void)right;
    (void)ctx;
    return false;
}

static enum traversal_cb_res typecheck_assignment(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;
    const struct type *final_type = NULL;

    // left side of an assignment should be an identifier or a variable declaration
    if (left->type != AST_IDENTIFIER &&
        left->type != AST_VARIABLE_DECLARATION &&
        !ast_node_is_specific_binaryop(left, BINARYOP_MEMBER_ACCESS)) {
        analyzer_err(
            ctx->m, ast_node_startmark(left),
            ast_node_endmark(left),
            "Expected an identifier a variable declaration, or a member access "
            "as left part of the assignment but found a \""RFS_PF"\"",
            RFS_PA(ast_node_str(left))
        );
        return TRAVERSAL_CB_ERROR;
    }


    if (!typecheck_binaryop_get_operands(n, left, &tleft, right, &tright, ctx)) {
        return TRAVERSAL_CB_ERROR;
    }

    typecmp_ctx_set_flags(TYPECMP_FLAG_ASSIGNMENT);
    if (type_compare(tright, tleft, TYPECMP_IMPLICIT_CONVERSION)) {
        final_type = tleft;
        if (typecmp_ctx_have_warning()) {
            const struct RFstring *warning;
            while ((warning = typecmp_ctx_get_next_warning())) {
                analyzer_warn(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                              RFS_PF" during assignment.",
                              RFS_PA(warning));
            }
        }

        // if the right side is a bracket list of an elementary array the
        // make sure that the constant types are upped to the integer level
        // of the left type
        if (right->type == AST_BRACKET_LIST && type_is_elementary_array(tright)) {
            typecheck_adjust_elementary_arr_const_values(right, tleft);
        }
    } else {
        if (!analyzer_types_assignable(left, right, ctx)) {
            RFS_PUSH();
            analyzer_err(
                ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                "Assignment between incompatible types. Can't assign "
                "\""RFS_PF"\" to \""RFS_PF"\"%s"RFS_PF".",
                RFS_PA(type_str_or_die(tright, TSTR_DEFAULT)),
                RFS_PA(type_str_or_die(tleft, TSTR_DEFAULT)),
                typecmp_ctx_have_error() ? ". " : "",
                RFS_PA(typecmp_ctx_get_error())
            );
            RFS_POP();
            return TRAVERSAL_CB_ERROR;
        }
        // temporary
        RF_ASSERT(false, "analyzer_types_assignable() should never return true for now");
    }

    // set the type of assignment as the type of the left operand
    traversal_node_set_type(n, final_type, ctx);
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_comma(
    struct ast_node *n,
    struct ast_node *left,
    struct ast_node *right,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *tright;
    const struct type *tleft;

    if (!typecheck_binaryop_get_operands(n, left, &tleft, right, &tright, ctx)) {
        return TRAVERSAL_CB_ERROR;
    }

    // for now at least let's assume that all types can have the comma
    // operator applied to them

    // create the comma type
    traversal_node_set_type(
        n,
        type_create_from_operation(TYPEOP_PRODUCT,
                                   n,
                                   (struct type*)tleft,
                                   (struct type*)tright,
                                   ctx->m),
        ctx
    );
    if (!ast_node_get_type(n)) {
        RF_ERROR("Could not create a type as a product of 2 other types.");
        return TRAVERSAL_CB_FATAL_ERROR;
    }

    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_return_stmt(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    struct ast_node *fn_decl = symbol_table_get_fndecl(ctx->current_st);
    const struct type *fn_type;
    if (!fn_decl) {
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "Return statement outside of function body");
        return TRAVERSAL_CB_ERROR;
    }

    fn_type = ast_node_get_type(fn_decl);
    if (!fn_type) {
        RF_ERROR("Function type should have been found here");
        return TRAVERSAL_CB_ERROR;
    }
    const struct type *fn_ret_type = type_function_get_rettype(fn_type);
    const struct type *found_ret_type = ast_node_get_type(ast_returnstmt_expr_get(n));

    if (!found_ret_type) {
        return TRAVERSAL_CB_ERROR;
    }

    if (!type_compare(fn_ret_type, found_ret_type, TYPECMP_IMPLICIT_CONVERSION)) {
        RFS_PUSH();
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "Return statement type \""RFS_PF"\" does not match "
                     "the expected return type of \""RFS_PF"\"",
                     RFS_PA(type_str_or_die(found_ret_type, TSTR_DEFAULT)),
                     RFS_PA(type_str_or_die(fn_ret_type, TSTR_DEFAULT)));
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }

    traversal_node_set_type(n, found_ret_type, ctx);
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_block(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    // a block's type is the type of it's last expression
    n->expression_type = ctx->last_node_type;
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_import(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    if (ast_import_is_foreign(n)) {
        module_add_foreign_import(ctx->m, n);
    }
    // import does not have a type
    n->expression_type = NULL;
    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_binaryop(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    struct ast_node *left = ast_binaryop_left(n);
    struct ast_node *right = ast_binaryop_right(n);
    enum binaryop_type bop_type = ast_binaryop_op(n);

    switch (bop_type) {
    case BINARYOP_ASSIGN:
        return typecheck_assignment(n, left, right, ctx);
    case BINARYOP_COMMA:
        return typecheck_comma(n, left, right, ctx);

    case BINARYOP_ADD:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't add", "to");
    case BINARYOP_SUB:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't subtract", "from");
    case BINARYOP_MUL:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't multiply", "by");
    case BINARYOP_DIV:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't divide", "by");

    case BINARYOP_CMP_EQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is equal to");
    case BINARYOP_CMP_NEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is not equal to");
    case BINARYOP_CMP_GT:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is greater than");
    case BINARYOP_CMP_GTEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is greater than or equal");
    case BINARYOP_CMP_LT:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is less than");
    case BINARYOP_CMP_LTEQ:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't compare if", "is less than or equal");

    case BINARYOP_LOGIC_AND:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply logic operator && between", "and");
    case BINARYOP_LOGIC_OR:
        return typecheck_bool_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply logic operator || between", "and");

    case BINARYOP_BITWISE_OR:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise OR to", "and");
    case BINARYOP_BITWISE_AND:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise AND to", "and");
    case BINARYOP_BITWISE_XOR:
        return typecheck_binaryop_generic(
            n, left, right, ctx, bop_type, i_should_be_changed,
            "Can't apply bitwise XOR to", "and");

    case BINARYOP_INDEX_ACCESS:
        return typecheck_indexaccess(n, left, right, ctx);
    case BINARYOP_MEMBER_ACCESS:
        return typecheck_member_access(n, left, right, ctx);

    default:
        RF_CRITICAL_FAIL(
            "Typechecking for unimplemented binary operator "RFS_PF,
            RFS_PA(ast_binaryop_opstr(n))
        );
        return TRAVERSAL_CB_FATAL_ERROR;
    }

    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_unaryop(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    enum unaryop_type uop_type = ast_unaryop_op(n);
    const struct type *operand_type;

    operand_type = ast_node_get_type(ast_unaryop_operand(n));
    if (!operand_type) {
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "Type for operand of \""RFS_PF"\" can not be determined",
                     RFS_PA(ast_unaryop_opstr(n)));
        return TRAVERSAL_CB_ERROR;
    }

    switch (uop_type) {
    case UNARYOP_MINUS:
        return typecheck_unaryop_generic(n, operand_type, "Can't apply", "to", ctx);
    case UNARYOP_PLUS:
        return typecheck_unaryop_generic(n, operand_type, "Can't apply", "to", ctx);
    case UNARYOP_INC:
        return typecheck_unaryop_generic(n, operand_type, "Can't apply", "to", ctx);
    case UNARYOP_DEC:
        return typecheck_unaryop_generic(n, operand_type, "Can't apply", "to", ctx);
    default:
        RF_CRITICAL_FAIL(
            "Typechecking for unimplemented unary operator "RFS_PF,
            RFS_PA(ast_unaryop_opstr(n))
        );
        return TRAVERSAL_CB_FATAL_ERROR;
    }

    return TRAVERSAL_CB_OK;
}

static enum traversal_cb_res typecheck_do(struct ast_node *n,
                                          void *user_arg)
{
    struct analyzer_traversal_ctx *ctx = (struct analyzer_traversal_ctx*)user_arg;
    enum traversal_cb_res ret = TRAVERSAL_CB_OK;
    switch(n->type) {
    case AST_BINARY_OPERATOR:
        ret = typecheck_binaryop(n, ctx);
        break;
    case AST_UNARY_OPERATOR:
        ret = typecheck_unaryop(n, ctx);
        break;
    case AST_BRACKET_LIST:
        ret = typecheck_bracketlist(n, ctx);
        break;
    case AST_IDENTIFIER:
        ret = typecheck_identifier(n, ctx);
        break;
    case AST_XIDENTIFIER:
        ret = typecheck_xidentifier(n, ctx);
        break;
    case AST_TYPE_DECLARATION:
        ret = typecheck_typedecl(n, ctx);
        break;
    case AST_TYPE_DESCRIPTION:
        ret = typecheck_typedesc(n, ctx);
        break;
    case AST_TYPE_OPERATOR:
        ret = typecheck_typeop(n, ctx);
        break;
    case AST_TYPE_LEAF:
        ret = typecheck_typeleaf(n, ctx);
        break;
    case AST_CONSTANT:
        ret = typecheck_constant(n, ctx);
        break;
    case AST_STRING_LITERAL:
        traversal_node_set_type(
            n,
            type_elementary_get_type_constant(ELEMENTARY_TYPE_STRING),
            ctx
        );
        break;
    case AST_VARIABLE_DECLARATION:
        // for a variable definition, the variable's type description should be
        // the type of the expression
        traversal_node_set_type(
            n,
            ast_node_get_type(ast_vardecl_desc_get(n)),
            ctx
        );
        break;
    case AST_FUNCTION_CALL:
        ret = typecheck_function_call(n, ctx);
        break;
    case AST_FUNCTION_DECLARATION:
        ret = typecheck_fndecl(n, ctx);
        break;
    case AST_FUNCTION_IMPLEMENTATION:
        traversal_node_set_type(
            n,
            ast_node_get_type(ast_fnimpl_fndecl_get(n)),
            ctx
        );
        break;
    case AST_RETURN_STATEMENT:
        ret = typecheck_return_stmt(n, ctx);
        break;
    case AST_BLOCK:
        ret = typecheck_block(n, ctx);
        break;
    case AST_MATCH_EXPRESSION:
        ret = typecheck_matchexpr(n, ctx);
        break;
    case AST_FOR_EXPRESSION:
        ret = typecheck_forexpr_ascending(n, ctx);
        break;
    case AST_ITERABLE:
        ret = typecheck_iterable(n, ctx);
        break;
    case AST_MATCH_CASE:
        ret = typecheck_matchcase(n, ctx);
        break;
    case AST_IMPORT:
        ret = typecheck_import(n, ctx);
        break;
    case AST_TYPECLASS_DECLARATION:
        ret = typecheck_typeclass(n, ctx);
        break;
    case AST_TYPECLASS_INSTANCE:
        ret = typecheck_typeinstance(n, ctx);
        break;
    default:
        // do nothing. Think what to do for the remaining nodes if anything ...
        break;
    }
    // also change symbol table upwards if needed
    analyzer_handle_symbol_table_ascending(n, ctx);
    return ret;
}

bool analyzer_typecheck(struct module *mod, struct ast_node *n)
{
    struct analyzer_traversal_ctx ctx;
    analyzer_traversal_ctx_init(&ctx, mod);

    bool ret = (TRAVERSAL_CB_OK == ast_traverse_tree_nostop_post_cb(
                    n,
                    (ast_node_cb)analyzer_handle_traversal_descending,
                    &ctx,
                    typecheck_do,
                    &ctx));

    analyzer_traversal_ctx_deinit(&ctx);
    return ret;
}


