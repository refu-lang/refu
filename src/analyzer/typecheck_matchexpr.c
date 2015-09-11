#include <analyzer/typecheck_matchexpr.h>

#include <module.h>
#include <ast/ast.h>
#include <ast/matchexpr.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>

#include <types/type_comparisons.h>
#include <types/type.h>

static inline bool pattern_matching_ctx_add_part(struct pattern_matching_ctx *ctx,
                                                 const struct type *t)
{
    return rf_objset_add(&ctx->parts, type, t);
}

static bool pattern_matching_ctx_populate_parts(struct pattern_matching_ctx *ctx,
                                                const struct type *t)
{
    switch (t->category) {
    case TYPE_CATEGORY_DEFINED:
        return pattern_matching_ctx_populate_parts(ctx, t->defined.type);
    case TYPE_CATEGORY_OPERATOR:
    {
        if (t->operator.type == TYPEOP_SUM) {
            const struct type *target_l = t->operator.left;
            const struct type *target_r = t->operator.right;
            if (target_l->category == TYPE_CATEGORY_LEAF) {
                target_l = target_l->leaf.type;
            }
            if (target_r->category == TYPE_CATEGORY_LEAF) {
                target_r = target_r->leaf.type;
            }
            // add the sum operands as different parts of the type in the set
            if (!pattern_matching_ctx_add_part(ctx, target_l)) {
                RF_ERROR("Internal error, could not add type to parts set.");
                return false;
            }
            if (!pattern_matching_ctx_add_part(ctx, target_r)) {
                RF_ERROR("Internal error, could not add type to parts set.");
                return false;
            }
            return true;
        } else {
            return pattern_matching_ctx_populate_parts(ctx, t->operator.left) &&
                pattern_matching_ctx_populate_parts(ctx, t->operator.right);
        }
    }
    case TYPE_CATEGORY_LEAF:
        return pattern_matching_ctx_populate_parts(ctx, t->leaf.type);
    case TYPE_CATEGORY_ELEMENTARY:
            return true;
    default:
        RF_CRITICAL_FAIL("should never happen");
        break;
    }

    return false;    
}

bool pattern_matching_ctx_init(struct pattern_matching_ctx *ctx,
                               const struct symbol_table *table,
                               struct ast_node *matchexpr)
{
    rf_objset_init(&ctx->parts, type);
    rf_objset_init(&ctx->matched, type);
    ctx->last_matched_case = NULL;
    ctx->match_is_over = false;

    // now set the matching type for the match expression and populate the parts set
    const struct type *matched_type = ast_matchexpr_matched_type_compute(matchexpr, table);
    if (!pattern_matching_ctx_populate_parts(ctx, matched_type)) {
        return false;
    }
    return true;
}

void pattern_matching_ctx_deinit(struct pattern_matching_ctx *ctx)
{
    rf_objset_clear(&ctx->parts);
    rf_objset_clear(&ctx->matched);
}

static bool pattern_matching_ctx_compare(struct pattern_matching_ctx *ctx,
                                         struct module *m,
                                         struct ast_node *matchexpr)
{
    struct rf_objset_iter it1;
    struct type *t1;
    struct rf_objset_iter it2;
    struct type *t2;
    bool found;
    bool ret = true;

    // manually check if PARTS is a subset of MATCHED, so that we can give nice errors
    // could have used @ref rf_objset_subset() and @ref rf_objset_equal()
    // or ... make a version of them that accepts a callback. TODO
    if (!rf_objset_empty(&ctx->parts)) {
        rf_objset_foreach(&ctx->parts, &it1, t1) {
            found = false;
            rf_objset_foreach(&ctx->matched, &it2, t2) {
                if (t1 == t2) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                RFS_PUSH();
                analyzer_err(m, ast_node_startmark(matchexpr),
                             ast_node_endmark(matchexpr),
                             "Match expression does not match all cases for "
                             "\""RF_STR_PF_FMT"\". Sum type operand of "
                             "\""RF_STR_PF_FMT"\" is not covered.",
                             RF_STR_PF_ARG(ast_matchexpr_matched_type_str(matchexpr)),
                             RF_STR_PF_ARG(type_str_or_die(t1, TSTR_DEFAULT)));
                RFS_POP();
                ret = false;
            }
        }
    }
    return ret;
}

static inline bool pattern_matching_ctx_set_matched(struct pattern_matching_ctx *ctx,
                                                    const struct type *match_type,
                                                    const struct type *pattern_type)
{
    if (rf_objset_get(&ctx->matched, type, match_type)) {
        return true;
    }
    if (pattern_type->category == TYPE_CATEGORY_WILDCARD || pattern_type->category == TYPE_CATEGORY_OPERATOR) {
        ctx->last_matched_case = match_type;
    } else {
        ctx->last_matched_case = pattern_type;
    }
    return rf_objset_add(&ctx->matched, type, match_type);
}


static bool pattern_match_type_operators(const struct type *pattern,
                                         const struct type *target,
                                         struct pattern_matching_ctx *ctx);

static bool pattern_match_types(const struct type *pattern,
                                const struct type *target,
                                struct pattern_matching_ctx *ctx)
{
    if (pattern->category == TYPE_CATEGORY_LEAF) {
        pattern = pattern->leaf.type;
    }

    switch (target->category) {
    case TYPE_CATEGORY_DEFINED:
        return pattern_match_types(pattern, target->defined.type, ctx);
    case TYPE_CATEGORY_OPERATOR:
        return pattern_match_type_operators(pattern, target, ctx);
    case TYPE_CATEGORY_LEAF:
        return pattern_match_types(pattern, target->leaf.type, ctx);
    case TYPE_CATEGORY_ELEMENTARY:
        if (pattern->category == TYPE_CATEGORY_WILDCARD ||
            (pattern->category == TYPE_CATEGORY_ELEMENTARY &&
             pattern->elementary.etype == target->elementary.etype)) {
            return true;
        }
        break;
    default:
        RF_CRITICAL_FAIL("generic or wildcard as target type should never occur");
        break;
    }

    return false;
}

static bool pattern_match_type_sumop(const struct type *pattern,
                                     const struct type *target,
                                     struct pattern_matching_ctx *ctx)
{
    const struct type *pattern_l = pattern;
    const struct type *pattern_r = pattern;
    const struct type *target_l = target->operator.left;
    const struct type *target_r = target->operator.right;
    bool left = false;
    bool right = false;
    if (target_l->category == TYPE_CATEGORY_LEAF) {
        target_l = target_l->leaf.type;
    }
    if (target_r->category == TYPE_CATEGORY_LEAF) {
        target_r = target_r->leaf.type;
    }

    if (pattern->category == TYPE_CATEGORY_OPERATOR &&
        pattern->operator.type == target->operator.type) {
        pattern_l = pattern->operator.left;
        pattern_r = pattern->operator.right;
    }

    left = pattern_match_types(pattern_l, target_l, ctx);
    right = pattern_match_types(pattern_r, target_r, ctx);
    if (left) {
        if (!pattern_matching_ctx_set_matched(ctx, target_l, pattern)) {
            RF_ERROR("Internal error, could not add type to matched set.");
            return false;
        }
    }
    if (right) {
        if (!pattern_matching_ctx_set_matched(ctx, target_r, pattern)) {
            RF_ERROR("Internal error, could not add type to matched set.");
            return false;
        }
    }
    return left || right;
}

static bool pattern_match_type_operators(const struct type *pattern,
                                         const struct type *target,
                                         struct pattern_matching_ctx *ctx)
{
    switch (target->operator.type) {
    case TYPEOP_PRODUCT:
    case TYPEOP_IMPLICATION:
        if (pattern->category == TYPE_CATEGORY_OPERATOR &&
            pattern->operator.type == target->operator.type) {
            return pattern_match_types(pattern->operator.left, target->operator.left, ctx) &&
                pattern_match_types(pattern->operator.right, target->operator.right, ctx);
        }
        // else it's a different operator or type so no match
        return false;
    case TYPEOP_SUM:
        return pattern_match_type_sumop(pattern, target, ctx);
    default:
        RF_CRITICAL_FAIL("This case should never occur");
        break;
    }
    return true;
}

enum traversal_cb_res typecheck_matchcase(struct ast_node *n, struct analyzer_traversal_ctx* ctx)
{
    enum traversal_cb_res ret = TRAVERSAL_CB_ERROR;
    if (ctx->matching_ctx.match_is_over) {
        // don't even bother doing any checks. Error was already
        // given for a previous case
        return TRAVERSAL_CB_ERROR;
    }
    bool useless_case = false;
    // before anything check if all parts have already been matched
    if (rf_objset_equal(&ctx->matching_ctx.matched, &ctx->matching_ctx.parts, type)) {
        // case is useless. Already an error but let's check if it would even match.
        // If it does not match it's a more important error and will be shown instead.
        useless_case = true;        
    }
    RFS_PUSH();
    const struct type *case_pattern_type = ast_node_get_type_or_die(
        ast_matchcase_pattern(n),
        AST_TYPERETR_DEFAULT
    );
    const struct type *match_type = ast_matchexpr_matched_type(
        analyzer_traversal_ctx_get_nth_parent_or_die(0, ctx)
    );
    const struct RFstring *match_type_str = ast_matchexpr_matched_type_str(
        analyzer_traversal_ctx_get_nth_parent_or_die(0, ctx)
    );
    // res_type can be NULL if the expression has void type. e.g.: print("foo")
    const struct type *res_type = ast_node_get_type_or_die(
        ast_matchcase_expression(n),
        AST_TYPERETR_DEFAULT
    );
    RF_ASSERT(case_pattern_type, "a type for the match case pattern should have been determined");

    if (!pattern_match_types(case_pattern_type, match_type, &ctx->matching_ctx)) {
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "Match case \""RF_STR_PF_FMT"\" can not be matched to the "
                     "type of \""RF_STR_PF_FMT"\" which is of type \""RF_STR_PF_FMT"\".",
                     RF_STR_PF_ARG(type_str_or_die(case_pattern_type, TSTR_DEFAULT)),
                     RF_STR_PF_ARG(match_type_str),
                     RF_STR_PF_ARG(type_str_or_die(match_type, TSTR_DEFINED_CONTENTS)));
        ctx->matching_ctx.match_is_over = true;
        goto end;
    }

    if (useless_case) {
        analyzer_err(ctx->m, ast_node_startmark(n), ast_node_endmark(n),
                     "Match case \""RF_STR_PF_FMT"\" is useless since all parts of "
                     "\""RF_STR_PF_FMT"\" have already been matched.",
                     RF_STR_PF_ARG(type_str_or_die(case_pattern_type, TSTR_DEFAULT)),
                     RF_STR_PF_ARG(match_type_str));
        ctx->matching_ctx.match_is_over = true;
        goto end;
    }
    // keep the type that this match case matched to
    n->matchcase.matched_type = ctx->matching_ctx.last_matched_case;
    traversal_node_set_type(n, res_type, ctx);
    ret = TRAVERSAL_CB_OK;

end:
    RFS_POP();
    return ret;
}

enum traversal_cb_res typecheck_matchexpr(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx)
{
    enum traversal_cb_res ret = TRAVERSAL_CB_OK;

    if (!pattern_matching_ctx_compare(&ctx->matching_ctx, ctx->m, n)) {
        ret = TRAVERSAL_CB_ERROR;
    }

    //set the type of the match expression as a SUM type of all its cases
    struct ast_matchexpr_it it;
    struct ast_node *mcase;
    struct rf_objset_type case_types;
    struct type *case_type;
    struct type *matchexpr_type = NULL;
    rf_objset_init(&case_types, type);
    ast_matchexpr_foreach(n, &it, mcase) {
        case_type = (struct type*)ast_node_get_type_or_die(
            ast_matchcase_expression(mcase),
            AST_TYPERETR_DEFAULT
        );
        // Check if it's in the set. If yes skip this.
        if (rf_objset_get(&case_types, type, case_type)) {
            continue;
        }
        // also check if it can be converted to a type already in the set and skip
        if (type_objset_has_convertable(&case_types, case_type)) {
            continue;
        }
        // in other case we add it to the set
        rf_objset_add(&case_types, type, case_type);
        // add to the type of the match expression itself
        if (matchexpr_type) {
            matchexpr_type = type_create_from_operation(
                TYPEOP_SUM,
                matchexpr_type,
                case_type,
                ctx->m);
        } else {
            matchexpr_type = case_type;
        }
    }

    rf_objset_clear(&case_types);
    traversal_node_set_type(n, matchexpr_type, ctx);
    pattern_matching_ctx_deinit(&ctx->matching_ctx);
    return ret;
}
