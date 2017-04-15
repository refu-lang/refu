#include <analyzer/typecheck_functions.h>

#include <rfbase/string/core.h>
#include <rfbase/string/conversion.h>

#include <ast/ast.h>
#include <ast/function.h>
#include <ast/constants.h>
#include <ast/operators.h>
#include <ast/typeclass.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <types/type.h>
#include <types/type_function.h>
#include <types/type_operators.h>
#include <types/type_comparisons.h>

enum traversal_cb_res typecheck_function_call(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *fn_type;
    const struct type *fn_declared_args_type;
    const struct type *fn_found_args_type;
    struct ast_node *fn_call_args = ast_fncall_args(n);
    const struct RFstring *fn_name = ast_fncall_name(n);
    struct type *self_type = NULL;
    struct ast_node *typeinstance_fnimpl = NULL;

    // check if this is a function call from a member access
    struct ast_node *parent = analyzer_traversal_ctx_get_nth_parent_or_die(0, ctx);
    if (ast_node_is_specific_binaryop(parent, BINARYOP_MEMBER_ACCESS)) {
        RF_ASSERT(
            ast_binaryop_right(parent) == n,
            "At the moment functions should only be on the right side of a "
            "member access operator."
        );
        struct ast_node *left = ast_binaryop_left(parent);
        const struct type *left_type = ast_node_get_type(left);
        // Now check which type classes are instantiated by the type
        // and see if they contain a function with the given name
        struct ast_node *typeinstance = module_search_type_instance(
            ctx->m,
            left_type
        );
        if (!typeinstance) {
            analyzer_err(
                ctx->m, ast_node_startmark(n),
                ast_node_endmark(n),
                "No typeclass instantiation for left type \""RFS_PF"\" found."
                "Can't call a method on a type without it.",
                RFS_PA(type_str_or_die(left_type, TSTR_DEFAULT))
            );
        }
        self_type = ast_typeinstance_instantiated_type_get(typeinstance);

        typeinstance_fnimpl = ast_typeinstance_getfn_byname(
            typeinstance,
            fn_name
        );
        if (!typeinstance_fnimpl) {
            analyzer_err(
                ctx->m, ast_node_startmark(n),
                ast_node_endmark(n),
                "Function \""RFS_PF"()\" was not defined in typeclass \""
                RFS_PF"\" instantiation for type \""RFS_PF"\".",
                RFS_PA(fn_name),
                ast_typeinstance_classname_str(typeinstance),
                ast_typeinstance_typename_str(typeinstance)
            );
        }
        fn_type = ast_node_get_type_or_die(ast_fnimpl_fndecl_get(typeinstance_fnimpl));
    } else {
        // non-member access function call,
        // check for presence of function declaration in context
        fn_type = type_lookup_identifier_string(fn_name, ctx->current_st);

        if (!fn_type || !type_is_callable(fn_type)) {
            analyzer_err(
                ctx->m, ast_node_startmark(n),
                ast_node_endmark(n),
                "Undefined function call \""RFS_PF"\" detected",
                RFS_PA(fn_name)
            );
            goto fail;
        }

        // also check the ast node of the function declaration to get more
        // information if it's not a conversion
        if (!type_is_explicitly_convertable_elementary(fn_type)) {
            const struct ast_node *fndecl = symbol_table_lookup_node(
                ctx->current_st,
                fn_name,
                NULL
            );
            RF_ASSERT(
                fndecl,
                "Since fn_type was found so should fndecl be found here"
            );
            if (fndecl->fndecl.position == FNDECL_PARTOF_FOREIGN_IMPORT) {
                n->fncall.type = AST_FNCALL_FOREIGN;
            }
        }
    }

    // create the arguments ast node array
    ast_fncall_arguments(n);

    fn_found_args_type = (fn_call_args)
        ? ast_node_get_type(fn_call_args)
        : type_elementary_get_type(ELEMENTARY_TYPE_NIL);

    if (!fn_found_args_type) { // argument typechecking failed
        goto fail;
    }
    // if we are in a function call of an instantiated typeclass
    if (typeinstance_fnimpl) {
        if (!ast_fnimpl_firstarg_is_self(typeinstance_fnimpl)) {
            analyzer_err(
                ctx->m, ast_node_startmark(n),
                ast_node_endmark(n),
                "Typeclass instantiation for function call \""RFS_PF"\" does "
                "not have 'self' as the first argument.",
                RFS_PA(fn_name)
            );
            goto fail;
        }

        // prepend the self type to the called arguments
        fn_found_args_type = type_create_from_operation(
            TYPEOP_PRODUCT,
            ast_fndecl_args_get(ast_fnimpl_fndecl_get(typeinstance_fnimpl)),
            self_type,
            (struct type*)fn_found_args_type,
            ctx->m
        );
    }

    if (type_is_explicitly_convertable_elementary(fn_type)) {
        // silly way to check if it's only 1 argument. Maybe figure out safer way?
        if (!fn_call_args || fn_found_args_type->category == TYPE_CATEGORY_OPERATOR) {
            analyzer_err(
                ctx->m,
                ast_node_startmark(n),
                ast_node_endmark(n),
                "Invalid arguments for explicit conversion to \""
                RFS_PF"\".",
                RFS_PA(fn_name)
            );
            goto fail;
        }

        // check if the explicit conversion is valid
        if (!type_compare(fn_found_args_type, fn_type, TYPECMP_EXPLICIT_CONVERSION)) {
            analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                         "Invalid explicit conversion. "RFS_PF".",
                         RFS_PA(typecmp_ctx_get_error()));
            goto fail;
        }
        n->fncall.type = AST_FNCALL_EXPLICIT_CONVERSION;
    } else {
        //check that the types of its arguments do indeed match
        fn_declared_args_type = type_callable_get_argtype(fn_type);
        n->fncall.declared_type = fn_declared_args_type;
        if (type_is_sumop(fn_declared_args_type)) {
            n->fncall.type = AST_FNCALL_SUM;
        }
        typecmp_ctx_set_flags(TYPECMP_FLAG_FUNCTION_CALL);
        if (!type_compare(
                fn_found_args_type,
                fn_declared_args_type,
                n->fncall.type == AST_FNCALL_SUM
                    ? TYPECMP_PATTERN_MATCHING :
                    TYPECMP_IMPLICIT_CONVERSION)) {
            RFS_PUSH();
            analyzer_err(
                ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                RFS_PF" "RFS_PF"() is called with argument type of \""RFS_PF
                "\" which does not match the expected "
                "type of \""RFS_PF"\"%s"RFS_PF".",
                RFS_PA(type_callable_category_str(fn_type)),
                RFS_PA(fn_name),
                RFS_PA(type_str_or_die(fn_found_args_type, TSTR_DEFAULT)),
                RFS_PA(type_str_or_die(fn_declared_args_type, TSTR_DEFINED_ONLY_CONTENTS)),
                typecmp_ctx_have_error() ? ". " : "",
                RFS_PA(typecmp_ctx_get_error())
            );
            RFS_POP();
            goto fail;
        }
        // the function call's matched type should be either a specific sum type
        // that may have matched or the entirety of the called arguments
        if (!(n->fncall.params_type = typemp_ctx_get_matched_type())) {
            n->fncall.params_type = fn_found_args_type;
        }
    }

    traversal_node_set_type(n, type_callable_get_rettype(fn_type), ctx);
    return TRAVERSAL_CB_OK;

fail:
    traversal_node_set_type(n, NULL, ctx);
    // TODO: At the moment of writting this comment if we put a non fatal error
    //       then we get assertion errors elsewhere in the code. Try to revert
    //       back to non-fatal error later and see if these are fixed.
    return TRAVERSAL_CB_FATAL_ERROR;
}

enum traversal_cb_res typecheck_fndecl(
    struct ast_node *n,
    struct analyzer_traversal_ctx *ctx)
{
    const struct type *t;
    t = type_lookup_identifier_string(ast_fndecl_name_str(n), ctx->current_st);
    if (!t) {
        RF_ERROR(
            "Function declaration name not found in the symbol table at "
            "impossible point"
        );
        return TRAVERSAL_CB_ERROR;
    }
    traversal_node_set_type(n, t, ctx);
    return TRAVERSAL_CB_OK;
}
