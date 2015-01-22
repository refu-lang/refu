#ifndef LFR_PARSER_FUNCTION_H
#define LFR_PARSER_FUNCTION_H

#include <stdbool.h>

struct parser;
struct ast_node;

#define TOKENS_ARE_FNDECL_OR_IMPL(tok1_, tok2_) \
    ((tok1_) && (tok2_) &&                      \
     (tok1_)->type == TOKEN_KW_FUNCTION  &&     \
     (tok2_)->type == TOKEN_IDENTIFIER)

/**
 * function_return = TOKEN_OP_IMPL type_description
 *
 * function_declaration = TOKEN_KW_FUNCTION identifier [generic_declaration]
 * TOKEN_SM_OPAREN type_description TOKEN_SM_CPAREN [function_return]
 *
 * @param fndecl_position      The position that the function declaration is
 *                             found in the code. @see enum fndecl_position for details.
 *
 */
struct ast_node *parser_acc_fndecl(struct parser *p, int fndecl_position);

enum parser_fndecl_list_err {
    PARSER_FNDECL_LIST_SUCCESS = 0,
    PARSER_FNDECL_LIST_EMPTY = 1,
    PARSER_FNDECL_LIST_FAILURE = 2,
};

/**
 * function_declarations = function_declaration
 *                       / function_declaration function_declarations
 *
 * Accepts a list of function declarations and adds them to another node
 *
 * @param p              The parser object to work with
 * @param parent         The ast_node on which to add the functions declarations
 * @param fndecl_position      The position that the function declaration is
 *                             found in the code. @see enum fndecl_position for details.
 *
 * @return               Will return an error code to determine if the list got
 *                       parsed succesfully, if there was an error or if it was
 *                       empty. @look parser_fndecl_list_err
 */
enum parser_fndecl_list_err parser_acc_fndecl_list(struct parser *p,
                                                   struct ast_node *parent,
                                                   int fndecl_position);


/*
 * function_implementation = function_declaration block
 */
struct ast_node *parser_acc_fnimpl(struct parser *p);

enum parser_fnimpl_list_err {
    PARSER_FNIMPL_LIST_SUCCESS = 0,
    PARSER_FNIMPL_LIST_EMPTY = 1,
    PARSER_FNIMPL_LIST_FAILURE = 2,
};

/**
 * function_implementations = function_implementation
 *                          / function_implementation function_implementations
 *
 * Accepts a list of function implementations and adds them to another node
 *
 * @param p              The parser object to work with
 * @param parent         The ast_node on which to add the function
 *                       implementations
 *
 * @return               Will return an error code to determine if the list got
 *                       parsed succesfully, if there was an error or if it was
 *                       empty. @look parser_fnimpl_list_err
 */
enum parser_fnimpl_list_err parser_acc_fnimpl_list(struct parser *p,
                                                   struct ast_node *parent);

#define TOKENS_ARE_POSSIBLE_FNCALL(tok1_, tok2_)                        \
    (tok1_ && tok2_ && (tok1_)->type == TOKEN_IDENTIFIER &&             \
     ((tok2_)->type == TOKEN_SM_OPAREN || (tok2_)->type == TOKEN_OP_LT))
/**
 * identifier [genrattr] TOKEN_SM_OPAREN expression TOKEN_SM_CPAREN
 */
struct ast_node *parser_acc_fncall(struct parser *p);

#endif
