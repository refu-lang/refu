#ifndef LFR_IR_FUNCTION
#define LFR_IR_FUNCTION

#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/datastructs/darray.h>
#include <rfbase/utils/container_of.h>

#include <ir/rir_strmap.h>
#include <ir/rir_argument.h>

struct ast_node;
struct rir_block;
struct rir_parser;
struct rir_ctx;
struct rir;
struct symbol_table;

struct rir_fndecl {
    //! Name of the function decl (owned)
    struct RFstring name;
    //! Array of the function's argument types
    struct rir_type_arr argument_types;
    //! Return type of the function
    struct rir_type *return_type;
    //! Differentiate between rir objects that are only with a declaration (true), and those with a body (false)
    bool plain_decl;
    //! Control to be entered into the rir functions list.
    struct RFilist_node ln;
};
struct rir_fndecl *rir_fndecl_create_from_ast(const struct ast_node *n, struct rir_ctx *ctx);

/**
 * Initialize a rir function declaration
 *
 * @param decl            The declaration to initialize
 * @param name            The name of the function
 * @param args            An array of types, denoting the arguments of the function,
 *                        function, or an empty type array with
 *                        darray_size() == 0 for no arguments.
 * @param return_type     The return type of the function. For no return type,
 *                        this has to be an ELEMENTARY_TYPE_NIL type.
 * @param foreign         True if this is just an fndecl and not a definition.
 * @param pos             The position in the compiler's RIR processing pipeline
 *                        for this function.
 * @param data            Extra rir data to be passed to this function
 */
bool rir_fndecl_init(
    struct rir_fndecl *decl,
    const struct RFstring *name,
    struct rir_type_arr *args,
    struct rir_type *return_type,
    bool foreign,
    enum rir_pos pos,
    rir_data data
);
/**
 * Create() version of rir_fndecl_init()
 */
struct rir_fndecl *rir_fndecl_create(
    const struct RFstring *name,
    struct rir_type_arr *args,
    struct rir_type *return_type,
    bool foreign,
    enum rir_pos pos,
    rir_data data
);

void rir_fndecl_destroy(struct rir_fndecl *f);
bool rir_fndecl_nocheck_tostring(struct rirtostr_ctx *ctx, bool is_plain, const struct rir_fndecl *f);
i_INLINE_DECL bool rir_fndecl_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *f)
{
    return rir_fndecl_nocheck_tostring(ctx, f->plain_decl, f);
}

struct rir_fndef {
    struct rir_fndecl decl;
    //! Rir object variables of the arguments of the function
    struct {darray(struct rir_object*);} variables;
    //! Array of all basic blocks under the function
    struct {darray(struct rir_block*);} blocks;
    //! Rir value of the return slot alloca. NULL if there is no return value.
    struct rir_value *retslot_val;
    //! Label pointing to the function's end
    struct rir_value *end_label;
    //! Stringmap from rir identifiers to rir objects
    struct rirobj_strmap map;
    //! Pointer to the symbol table of the function's arguments
    struct symbol_table *st;
};

struct rir_fndef *rir_fndef_create_from_ast(const struct ast_node *n, struct rir_ctx *ctx);
/**
 * Initialize a function definition, without touching the declaration part.
 * The declation member of @a def must have already been initialized.
 */
bool rir_fndef_init_no_decl(
    struct rir_fndef *def,
    enum rir_pos pos,
    rir_data data
);
struct rir_fndef *rir_fndef_create_no_decl(enum rir_pos pos, rir_data data);

void rir_fndef_destroy(struct rir_fndef *f);

/**
 * Given a rir value check if it corresponds to an argument of the function
 * @param f        The function to check
 * @param v        The rir value to check whether or not is an argument
 *                 to the function
 * @return         The number of the corresponding argument, or -1 for not found
 */
int rir_fndef_value_to_argnum(const struct rir_fndef *f, const struct rir_value *v);
void rir_fndef_add_block(struct rir_fndef *f, struct rir_block *b);
bool rir_fndef_tostring(struct rirtostr_ctx *ctx, const struct rir_fndef *f);
i_INLINE_DECL struct rir_fndef *rir_fndecl_to_fndef(const struct rir_fndecl* d)
{
    return container_of(d, struct rir_fndef, decl);
}

/* -- Generic Methods that will work for both declaration and definitions -- */

bool rir_function_tostring(struct rirtostr_ctx *ctx, const struct rir_fndecl *fn);
void rir_function_destroy(struct rir_fndecl *d);
#endif
