#include "astprinter.h"
#include <json-c/json.h>
#include <ast/ast.h>


struct astprinter {
    json_object *root;
    json_object *current;
};

static bool astprinter_init(struct astprinter *p)
{
    p->root = json_object_new_object();
    json_object *ast = json_object_new_array();
    json_object_object_add(p->root ,"AST", ast);
    p->current = ast;
    return p->root;
}

static json_object *astprinter_json_new(const char *type)
{
    json_object *ret = json_object_new_object();
    
    json_object *typejstr = json_object_new_string(type);
    json_object_object_add(ret, "type", typejstr);
    

    return ret;
}

static bool astprinter_handle_node(struct astprinter *p,
                                   const struct ast_node *n)
{
    json_object *new_json = NULL;
    json_object *old_current = p->current;
    switch(n->type) {
    case AST_FUNCTION_IMPLEMENTATION:
        new_json = astprinter_json_new("functionimpl");
        break;
    case AST_IDENTIFIER:
        new_json = astprinter_json_new("identifier");
        break;
    default:
        
        break;
    }

    if (ast_node_get_children_number(n) != 0) {
        json_object *json_children = json_object_new_array();
        struct ast_node *child;
        rf_ilist_for_each(&n->children, child, lh) {
            p->current = json_children;
            if (!astprinter_handle_node(p, child)) {
                return false;
            }
        }
        json_object_object_add(new_json, "children", json_children);
        p->current = old_current;
    }

    if (new_json) {
        json_object_array_add(p->current, new_json);
    }
    
    return true;
}

bool ast_output_to_file(const struct ast_node *root, FILE *f)
{
    struct astprinter p;
    if (!astprinter_init(&p)) {
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
