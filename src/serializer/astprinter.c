#include "astprinter.h"
#include <json-c/json.h>
#include <ast/ast.h>
#include <inpfile.h>


struct astprinter {
    json_object *root;
    json_object *current;
    const struct inpfile *infile;
};

static bool astprinter_init(struct astprinter *p, const struct inpfile *infile)
{
    p->root = json_object_new_object();
    json_object *ast = json_object_new_array();
    json_object_object_add(p->root ,"AST", ast);
    p->current = ast;
    p->infile = infile;
    return p->root;
}

static json_object *astprinter_json_new(const struct astprinter *p,
                                        const struct ast_node *n,
                                        const char *type)
{
    const struct inplocation_mark *start_mark = ast_node_startmark(n);
    const struct inplocation_mark *end_mark = ast_node_endmark(n);
    json_object *ret = json_object_new_object();
    
    json_object *typejstr = json_object_new_string(type);
    json_object_object_add(ret, "type", typejstr);
    json_object *locstart = json_object_new_int64(inpfile_ptr_to_offset(p->infile, start_mark->p));
    json_object_object_add(ret, "location_start", locstart);
    json_object *locend = json_object_new_int64(inpfile_ptr_to_offset(p->infile, end_mark->p));
    json_object_object_add(ret, "location_end", locend);
    
    return ret;
}

static bool astprinter_handle_node(struct astprinter *p,
                                   const struct ast_node *n)
{
    json_object *new_json = NULL;
    json_object *old_current = p->current;
    switch(n->type) {
    case AST_FUNCTION_IMPLEMENTATION:
        new_json = astprinter_json_new(p, n, "functionimpl");
        break;
    case AST_IDENTIFIER:
        new_json = astprinter_json_new(p, n, "identifier");
        break;
    default:
        
        break;
    }

    if (ast_node_get_children_number(n) != 0) {
        json_object *json_children;
        if (new_json) {
            json_children = json_object_new_array();
        }
        struct ast_node *child;
        rf_ilist_for_each(&n->children, child, lh) {
            if (new_json) {
                p->current = json_children;
            }
            if (!astprinter_handle_node(p, child)) {
                return false;
            }
        }

        if (new_json) {
            json_object_object_add(new_json, "children", json_children);
        }
        p->current = old_current;
    }

    if (new_json) {
        json_object_array_add(p->current, new_json);
    }
    
    return true;
}

bool ast_output_to_file(const struct ast_node *root,
                        FILE *f,
                        const struct inpfile *inf)
{
    struct astprinter p;
    if (!astprinter_init(&p, inf)) {
        return false;
    }

    struct ast_node *child;
    rf_ilist_for_each(&root->children, child, lh) {
        if (!astprinter_handle_node(&p, child)) {
            return false;
        }
    }

    fprintf(f, "%s", json_object_to_json_string(p.root));
    fflush(f);

    return true;
}
