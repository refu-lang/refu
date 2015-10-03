#ifndef LFR_IR_RIR_VALUE_H
#define LFR_IR_RIR_VALUE_H

#include <Definitions/inline.h>
#include <Data_Structures/darray.h>
#include <String/rf_str_decl.h>
#include <ast/constants_decls.h>

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
    struct rir_ltype *type;
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
 * @param v            The value to initialize
 * @param obj          The rir object containing the value
 * @param type         An optional type to give to the value. If this is not
 *                     given then the value has to be determined by the rir
 *                     object itself.
 * @param ctx          The rir context
 * @return             True for succes, false otherwise
 */
bool rir_value_variable_init(struct rir_value *v, struct rir_object *obj, struct rir_ltype *type, struct rir_ctx *ctx);
bool rir_value_literal_init(struct rir_value *v, struct rir_object *obj, const struct RFstring *name, const struct RFstring *value);
bool rir_value_label_init_string(struct rir_value *v, struct rir_object *obj, const struct RFstring *s, struct rir_ctx *ctx);
bool rir_value_label_init(struct rir_value *v, struct rir_object *obj, bool function_beginning, struct rir_ctx *ctx);
bool rir_value_constant_init(struct rir_value *v, const struct ast_constant *c);
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
 * temporary string.
 */
const struct RFstring *rir_value_actual_string(const struct rir_value *v);

int64_t rir_value_constant_int_get(const struct rir_value *v);

i_INLINE_DECL bool rir_value_is_nil(const struct rir_value *v)
{
    return v->category == RIR_VALUE_NIL;
}
#endif
