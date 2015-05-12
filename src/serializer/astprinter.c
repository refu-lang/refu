#include "astprinter.h"
#include <json-c/json.h>
#include <ast/ast.h>
#include <inpfile.h>


struct astprinter {
    json_object *root;
    json_object *current;
    const struct inpfile *infile;
};

static bool astprinter_handle_node(struct astprinter *p,
                                   const struct ast_node *n);

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
    json_object_object_add(ret, "start", locstart);
    json_object *locend = json_object_new_int64(inpfile_ptr_to_offset(p->infile, end_mark->p));
    json_object_object_add(ret, "end", locend);
    
    return ret;
}

#if 0
static json_object *astprinter_typedesc_new(const struct astprinter *p,
                                            const struct ast_node *n)
{
    json_object *ret;
    ret = astprinter_json_new(p, n, "typedesc");
    json_object_object_add(ret, "left", astprinter_json_new(p, n->typeleaf.left, "identifier"));
    if (n->typeleaf.right->type != AST_IDENTIFIER || n->typeleaf.right->type != AST_XIDENTIFIER) {
        json_object_object_add(ret, "right", astprinter_json_new(p, n->typeleaf.right, "identifier"));
    } else {
        json_object_object_add(ret, "right", astprinter_json_new(p, n->typeleaf.right, "identifier"));
    }
    return ret;
}

static json_object *astprinter_typeleaf_new(const struct astprinter *p,
                                            const struct ast_node *n)
{
    json_object *ret;
    ret = astprinter_json_new(p, n, "typeleaf");
    json_object_object_add(ret, "left", astprinter_json_new(p, n->typeleaf.left, "identifier"));
    if (n->typeleaf.right->type != AST_IDENTIFIER || n->typeleaf.right->type != AST_XIDENTIFIER) {
        json_object_object_add(ret, "right", astprinter_json_new(p, n->typeleaf.right, "identifier"));
    } else {
        json_object_object_add(ret, "right", astprinter_json_new(p, n->typeleaf.right, "identifier"));
    }
    return ret;
}
#endif

static bool astprinter_handle_node(struct astprinter *p,
                                   const struct ast_node *n)
{
    json_object *new_json = NULL;
    json_object *old_current = p->current;
    json_object *json_children;
    bool visit_children = true;
    switch (n->type) {
    case AST_FUNCTION_IMPLEMENTATION:
        new_json = astprinter_json_new(p, n, "functionimpl");
        break;
    case AST_IDENTIFIER:
        new_json = astprinter_json_new(p, n, "identifier");
        break;
    case AST_TYPE_LEAF:
        /* new_json = astprinter_typeleaf_new(p, n); */
        /* visit_children = false; */
        new_json = astprinter_json_new(p, n, "typeleaf");
        break;
    case AST_TYPE_DESCRIPTION:
        new_json = astprinter_json_new(p, n, "typedesc");
        break;
    case AST_TYPE_OPERATOR:
        new_json = astprinter_json_new(p, n, "typeop");
        break;
    default:
        break;
    }

    if (visit_children && ast_node_get_children_number(n) != 0) {
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
