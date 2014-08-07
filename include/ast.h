#include <RFintrusive_list.h>
#include <RFstring.h>

struct parser_file;
struct parser_string;

struct ast_location {
    struct parser_file *file;
    unsigned int start_line;
    unsigned int start_col;
    unsigned int end_line;
    unsigned int end_col;
    char *sp;
    char *ep;
};

#define AST_LOC_INIT(file_, sl_, scol_, el_, ecol_, sp_, ep_)           \
    {.file = (file_), .start_line = (sl_), .start_col = (scol_),        \
        .end_line = (el_), end_col = (ecol_), sp = (sp_), ep = (ep_) }

#define AST_LOC_PARSER_INIT(file_)                      \
    {.file = (file_), .start_line = 0, .start_col = 0,  \
        .end_line = 0, end_col = 0, sp = 0, ep = 0}

/**
 * Initialize an ast_location from a parser string.
 *
 * @param loc       The location to initialize
 * @param p         The parser string from which to initialize the location
 * @param f         The parser file that contains the location
 * @param sp        Pointer at location start
 * @param eindex    Pointer at location end
 *
 * @return          True/false depending on success/failure
 */
bool ast_location_from_parserstr(struct ast_location *loc,
                                 struct parser_string *p,
                                 struct parser_file *f,
                                 char *sp, char *ep);

i_INLINE_DECL void ast_location_copy(struct ast_location *l1,
                                     struct ast_location *l2)
{
    l1->file = l2->file;
    l1->start_line = l2->start_line;
    l1->start_col = l2->start_col;
    l1->end_line = l2->end_line;
    l1->end_col = l2->end_col;
    l1->sp = l2->sp;
    l1->ep = l2->ep;
}

enum ast_type {
    AST_ROOT,
    AST_BLOCK,
    AST_IDENTIFIER,
    AST_VARIABLE_DECLARATION,

    /* from this value and up all types should have no children */
    AST_LEAVES,
    AST_STRING_LITERAL,
};

struct ast_node {
    enum ast_type type;
    struct ast_location location;
    struct RFilist_node lh;
    union {
        struct RFstring value_identifier;
        struct {
            struct RFilist_head children;
            unsigned int children_num;
        };
    };
};


struct ast_node *ast_node_create(enum ast_type type,
                                 struct ast_location *loc);
void ast_node_destroy(struct ast_node *n);

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child);


