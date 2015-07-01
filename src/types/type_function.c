#include <types/type_function.h>

#include <module.h>
#include <types/type.h>
#include <String/rf_str_core.h>

i_INLINE_INS bool type_is_function(const struct type *t);
i_INLINE_INS bool type_is_callable(const struct type *t);
i_INLINE_INS struct type *type_callable_get_argtype(const struct type *t);
i_INLINE_INS void type_function_set_argtype(struct type *t, struct type *other);
i_INLINE_INS struct type *type_function_get_rettype(const struct type *t);
i_INLINE_INS struct type *type_callable_get_rettype(const struct type *t);
i_INLINE_INS void type_function_set_rettype(struct type *t, struct type *other);

static const struct RFstring s_function_ = RF_STRING_STATIC_INIT("function");
static const struct RFstring s_ctor_ = RF_STRING_STATIC_INIT("constructor");
static const struct RFstring sforeign_pint64 = RF_STRING_STATIC_INIT("rf_stdlib_print_int64");
static const struct RFstring sforeign_puint64 = RF_STRING_STATIC_INIT("rf_stdlib_print_uint64");
static const struct RFstring sforeign_pdouble = RF_STRING_STATIC_INIT("rf_stdlib_print_double");
static const struct RFstring sforeign_pstr = RF_STRING_STATIC_INIT("rf_stdlib_print_string");

struct type *type_function_get_argtype(const struct type *t)
{
    RF_ASSERT(type_is_function(t), "Non function type detected");
    if (t->category == TYPE_CATEGORY_FOREIGN_FUNCTION) {
        // should allow only same foreign functions as in type_foreign_function_allowed()
        if (rf_string_equal(t->foreignfn.name, &sforeign_pint64)) {
            return type_elementary_get_type(ELEMENTARY_TYPE_INT_64);
        } else if (rf_string_equal(t->foreignfn.name, &sforeign_puint64)) {
            return type_elementary_get_type(ELEMENTARY_TYPE_UINT_64);
        } else if (rf_string_equal(t->foreignfn.name, &sforeign_pdouble)) {
            return type_elementary_get_type(ELEMENTARY_TYPE_FLOAT_64);
        } else if (rf_string_equal(t->foreignfn.name, &sforeign_pstr)) {
            return type_elementary_get_type(ELEMENTARY_TYPE_STRING);
        } else {
            RF_ASSERT_OR_EXIT(false, "Attempted to request args of illegal foreign function");
            return NULL;
        }
    }
    return t->operator.left;
}

const struct RFstring *type_callable_category_str(const struct type *t)
{
    RF_ASSERT(type_is_callable(t), "Non callable type detected");
    if (type_is_function(t)) {
        return &s_function_;
    }
    return &s_ctor_;
}

static inline bool type_is_product_op(const struct type *t)
{
    return t->category == TYPE_CATEGORY_OPERATOR && t->operator.type == TYPEOP_PRODUCT;
}

static const struct type *do_type_fnargs_get_argtype_n(const struct type_operator *op,
                                                       unsigned int n)
{
    const struct type *t;
    if (n == 0) {
        t = op->left;
        // if it's a leaf don't take the identifier
        if (t->category == TYPE_CATEGORY_LEAF) {
            return t->leaf.type;
        }
        return NULL;
    } else {
        t = op->right;
        // special case of only 2 arguments
        if (n == 1 && t->category != TYPE_CATEGORY_LEAF) {
            return NULL;
        }
        return t->leaf.type;
    }
    RF_ASSERT_OR_CRITICAL(type_is_product_op(t),
                          return NULL,
                          "function call arguments can only be comma separated");

    // continue recursion
    return do_type_fnargs_get_argtype_n(&t->operator, n - 1);
}

void type_function_init(struct type *t, struct type *arg_type, struct type *ret_type)
{
    t->category = TYPE_CATEGORY_OPERATOR;
    t->operator.type = TYPEOP_IMPLICATION;

    type_function_set_argtype(t, arg_type);
    type_function_set_rettype(t, ret_type);
}

const struct type *type_fnargs_get_argtype_n(const struct type *t, unsigned int n)
{
    // special case of 1 argument only
    if (n == 0 && t->category == TYPE_CATEGORY_LEAF) {
        return t->operator.left->leaf.type;
    }
    RF_ASSERT_OR_CRITICAL(type_is_product_op(t),
                          return NULL,
                          "function call arguments can only be comma separated");

    return do_type_fnargs_get_argtype_n(&t->operator, n);
}


struct type *type_foreign_function_create(struct module *mod, const struct RFstring *name)
{
    struct type *ret;
    ret = type_alloc(mod);
    if (!ret) {
        RF_ERROR("Type allocation failed");
        return NULL;
    }

    ret->category = TYPE_CATEGORY_FOREIGN_FUNCTION;
    ret->foreignfn.name = name;
    // do not add foreign function type to the types list at the moment
    /* analyzer_types_set_add(mod, ret); */
    return ret;
}

bool type_foreign_function_allowed(const struct type *t)
{    
    return rf_string_equal(t->foreignfn.name, &sforeign_pint64)    ||
           rf_string_equal(t->foreignfn.name, &sforeign_puint64)   ||
           rf_string_equal(t->foreignfn.name, &sforeign_pdouble)   ||
           rf_string_equal(t->foreignfn.name, &sforeign_pstr);
}

i_INLINE_INS bool type_is_foreign_function(const struct type *t);
