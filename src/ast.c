#include <ast.h>
#include <RFmemory.h>

static inline void ast_location_copy(struct ast_location *from,
                                     struct ast_location *to)
{
    to->file = from->file;
    to->start_line = from->start_line;
    to->start_col = from->start_col;
    to->end_line = from->end_line;
    to->end_col = from->end_col;
}


struct ast_node *ast_node_new(enum ast_type type,
                              struct parser_file *pfile,
                              char *sp, char *ep)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), NULL);

    ret->type = type;
    rf_ilist_head_init(&ret->children);

    ret->location.file = file;
    ret->location.start_line = start_line;
    ret->location.start_col = start_col;
    ret->location.beg = sp;

    ret->location.file = pfile;
    ret->location.start_line = pfile->offset.lines_moved;
    //TODO: column
    ret->location.beg = sp;
    ret->location.end = ep;

    return ret;
}

void ast_node_add_child(struct ast_node *parent,
                        struct ast_node *child)
{
    rf_ilist_add(&parent->children, &child->lh);
    parent->children_num ++;
}
