#include <analyzer/typecheck_matchexpr.h>

#include <ast/ast.h>
#include <ast/matchexpr.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>

#include <types/type_comparisons.h>
#include <types/type.h>

void pattern_matching_ctx_init(struct pattern_matching_ctx *ctx) {
    rf_objset_init(&ctx->parts, type);
    rf_objset_init(&ctx->matched, type);
}

void pattern_matching_ctx_deinit(struct pattern_matching_ctx *ctx) {
    rf_objset_clear(&ctx->parts);
    rf_objset_clear(&ctx->matched);
}

static bool pattern_matching_ctx_compare(struct pattern_matching_ctx *ctx,
                                         struct analyzer *analyzer,
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
                analyzer_err(analyzer, ast_node_startmark(matchexpr),
                             ast_node_endmark(matchexpr),
                             "Match expression does not match all cases for "
                             "\""RF_STR_PF_FMT"\". Sum type operand of "
                             "\""RF_STR_PF_FMT"\" is not covered.",
                             RF_STR_PF_ARG(ast_identifier_str(ast_matchexpr_identifier(matchexpr))),
                             RF_STR_PF_ARG(type_str_or_die(t1, TSTR_DEFAULT)));
                RFS_POP();
                ret = false;
            }
        }
    }
    return ret;
}

static inline bool pattern_matching_ctx_add_part(struct pattern_matching_ctx *ctx,
                                                 const struct type *t)
{
    return rf_objset_add(&ctx->parts, type, t);
}

static inline bool pattern_matching_ctx_set_matched(struct pattern_matching_ctx *ctx,
                                                    const struct type *match_type)
{
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
        RF_ASSERT_OR_EXIT(false, "generic or wildcard as target type should never occur");
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
    // add the sum operands as different parts of the type in the set
    if (!pattern_matching_ctx_add_part(ctx, target_l)) {
        RF_ERROR("Internal error, could not add type to parts set.");
        return false;
    }
    if (!pattern_matching_ctx_add_part(ctx, target_r)) {
        RF_ERROR("Internal error, could not add type to parts set.");
        return false;
    }

    if (pattern->category == TYPE_CATEGORY_OPERATOR &&
        pattern->operator.type == target->operator.type) {
        pattern_l = pattern->operator.left;
        pattern_r = pattern->operator.right;
    }

    left = pattern_match_types(pattern_l, target_l, ctx);
    if (left) {
        if (!pattern_matching_ctx_set_matched(ctx, target_l)) {
            RF_ERROR("Internal error, could not add type to matched set.");
            return false;
        }
    }
    right = pattern_match_types(pattern_r, target_r, ctx);
    if (right) {
        if (!pattern_matching_ctx_set_matched(ctx, target_r)) {
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
        RF_ASSERT_OR_EXIT(false, "This case should never occur");
        break;
    }
    return true;
}

enum traversal_cb_res typecheck_matchcase(struct ast_node *n, struct analyzer_traversal_ctx* ctx)
{
    struct ast_node *parent_matchexpr_id = ast_matchexpr_identifier(
        analyzer_traversal_ctx_get_nth_parent(0, ctx)
    );
    const struct type *case_pattern_type = ast_matchcase_pattern(n)->expression_type;
    const struct type *match_type = parent_matchexpr_id->expression_type;
    const struct type *res_type = ast_matchcase_expression(n)->expression_type;
    RF_ASSERT(case_pattern_type, "a type for the match case should have been determined");

    if (!pattern_match_types(case_pattern_type, match_type, &ctx->matching_ctx)) {
        RFS_PUSH();
        analyzer_err(ctx->a, ast_node_startmark(n), ast_node_endmark(n),
                     "Match case \""RF_STR_PF_FMT"\" can not be matched to the "
                     "type of \""RF_STR_PF_FMT"\" which is of type \""RF_STR_PF_FMT"\".",
                     RF_STR_PF_ARG(type_str_or_die(case_pattern_type, TSTR_DEFAULT)),
                     RF_STR_PF_ARG(ast_identifier_str(parent_matchexpr_id)),
                     RF_STR_PF_ARG(type_str_or_die(match_type, TSTR_DEFINED_CONTENTS)));
        RFS_POP();
        return TRAVERSAL_CB_ERROR;
    }
    RF_ASSERT(res_type, "Type of a match case's expression was not determined.");
    traversal_node_set_type(n, res_type, ctx);
    return TRAVERSAL_CB_OK;
}

enum traversal_cb_res typecheck_matchexpr(struct ast_node *n,
                                          struct analyzer_traversal_ctx *ctx)
{
    enum traversal_cb_res ret = TRAVERSAL_CB_OK;

    if (!pattern_matching_ctx_compare(&ctx->matching_ctx, ctx->a, n)) {
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
        RF_ASSERT(ast_matchcase_expression(mcase)->expression_type,
                  "A match case's expression type is not determined");
        case_type = (struct type*)ast_matchcase_expression(mcase)->expression_type;
        // Check if it's in the set. If yes skip this. If not add it
        if (rf_objset_get(&case_types, type, case_type)) {
            continue;
        }
        rf_objset_add(&case_types, type, case_type);
        // add to the type of the match expression itself
        if (matchexpr_type) {
            matchexpr_type = type_create_from_operation(
                TYPEOP_SUM,
                matchexpr_type,
                case_type,
                ctx->a);
        } else {
            matchexpr_type = case_type;
        }
    }

    rf_objset_clear(&case_types);
    traversal_node_set_type(n, matchexpr_type, ctx);
    pattern_matching_ctx_deinit(&ctx->matching_ctx);
    return ret;
}
