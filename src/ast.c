#include <ast.h>
#include <RFmemory.h>

static inline void ast_location_copy(struct ast_location *from,
                                     struct ast_location *to)
{
    to->file_name = from->file_name;
    to->start_line = from->start_line;
    to->start_col = from->start_col;
    to->end_line = from->end_line;
    to->end_col = from->end_col;
}


struct ast_node *ast_node_new(enum ast_type type,
                              struct ast_location *location)
{
    struct ast_node *ret;
    RF_MALLOC(ret, sizeof(struct ast_node), NULL);
    
    ret->type = type;
    ast_location_copy(location, &ret->location);

    return ret;
}



