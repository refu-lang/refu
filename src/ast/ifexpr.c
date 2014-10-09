#include <ast/ast.h>
#include <ast/ifexpr.h>

struct ast_node *ast_condbranch_create(struct inplocation_mark *start,
                                       struct inplocation_mark *end,
                                       struct ast_node *cond,
                                       struct ast_node *body)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_CONDITIONAL_BRANCH, start, end);
    if (!ret) {
        return NULL;
    }
   
    ast_node_register_child(ret, cond, condbranch.cond);
    ast_node_register_child(ret, body, condbranch.body);

    return ret;
}

struct ast_node *ast_ifexpr_create(struct inplocation_mark *start,
                                   struct inplocation_mark *end,
                                   struct ast_node *taken_branch,
                                   struct ast_node *fall_through_branch)
{
    struct ast_node *ret;
    ret = ast_node_create_marks(AST_IF_EXPRESSION, start, end);
    if (!ret) {
        return NULL;
    }
   
    ast_node_register_child(ret, taken_branch, ifexpr.taken_branch);
    ast_node_register_child(ret, fall_through_branch, ifexpr.fall_through_branch);

    return ret;    
    
}

