#ifndef LFR_IR_RIR_VALUE_H
#define LFR_IR_RIR_VALUE_H

#include <Definitions/inline.h>
#include <Data_Structures/darray.h>
#include <String/rf_str_decl.h>
#include <ast/constants_decls.h>
#include <types/type_decls.h>
#include <ir/rir_common.h>

struct rir;
struct rir_ctx;
struct rir_expression;

enum rir_valtype {
    RIR_VALUE_CONSTANT,
    RIR_VALUE_VARIABLE,
    RIR_VALUE_LABEL,
    RIR_VALUE_LITERAL,
    RIR_VALUE_NIL
};

struct rir_value {
    //! General category of this value
    enum rir_valtype category;
    //! A string to represent the value in the generated IR file
    struct RFstring id;
    //! The type of the value. Or NULL if this is a label or a NIL value
    struct rir_type *type;
    union {
        struct ast_constant constant;
        struct rir_object *obj;
        struct rir_block *label_dst;
        struct RFstring literal;
    };
};
//! An array of values
struct value_arr {darray(struct rir_value*);};

/**
 * Initialize the value of a variable
 *
 * The identifier's value depends on the position in the code from which
 * the function is called. If it's called from the AST then it's an ever
 * increasing number like, $2, $3 e.t.c.
 *
 * If called from Parsing the rir_pctx (which should be the rir_data) should
 * contain the string value.
 *
 * @param v            The value to initialize
 * @param obj          The rir object containing the value
 * @param type         An optional type to give to the value. If this is not
 *                     given then the value has to be determined by the rir
 *                     object itself.
 * @param pos          Denotes which code module the function is called form.
 * @param data         The rir data. If @a pos:
 *                         + RIRPOS_AST   -> then this is rir_ctx
 *                         + RIRPOS_PARSE -> then this is rir_pctx
 * @return             True for succes, false otherwise
 */
bool rir_value_variable_init(
    struct rir_value *v,
    struct rir_object *obj,
    struct rir_type *type,
    enum rir_pos pos,
    rir_data data
);

bool rir_value_literal_init(struct rir_value *v, struct rir_object *obj, const struct RFstring *name, const struct RFstring *value);
bool rir_value_label_init_string(
    struct rir_value *v,
    struct rir_object *obj,
    const struct RFstring *s,
    struct rir_common *common
);

/**
 * Initialize a constant value
 * @param v        The value to initialize
 * @param c        The ast_constant from whicn to populate the value
 * @param type     Optional type information regarding the constant. This allows us
 *                 to have constants smaller than i64 or f64. If not given
 *                 (value = ELEMENTARY_TYPE_TYPES_COUNT) then the
 *                 biggest type for the constant is used.
 * @return         true for success
 */
bool rir_value_constant_init(struct rir_value *v, const struct ast_constant *c, enum elementary_type type);
void rir_value_nil_init(struct rir_value *v);

void rir_value_deinit(struct rir_value *v);
void rir_value_destroy(struct rir_value *v);

struct rir_block *rir_value_label_dst(const struct rir_value *v);
/**
 * Get a string representation of the rir value's identifier
 */
bool rir_value_tostring(struct rir *r, const struct rir_value *v);
const struct RFstring *rir_value_string(const struct rir_value *v);
/**
 * Get a string representation of the actual value of the rir value
 * This may be possible only for a few value types. For the rest return NULL.
 *
 * Needs to be enclosed in RFS_PUSH() and RFS_POP() since it may create a
 * temporary string. If you know the value is that of a string literal then
 * enclosing is not needed.
 */
const struct RFstring *rir_value_actual_string(const struct rir_value *v);

int64_t rir_value_constant_int_get(const struct rir_value *v);

i_INLINE_DECL bool rir_value_is_nil(const struct rir_value *v)
{
    return v->category == RIR_VALUE_NIL;
}
#endif
