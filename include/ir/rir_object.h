#ifndef LFR_IR_RIR_OBJECT_H
#define LFR_IR_RIR_OBJECT_H

#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir_block.h>
#include <ir/rir_typedef.h>
#include <ir/rir_global.h>
#include <ir/rir_variable.h>
#include <RFintrusive_list.h>
#include <Utils/sanity.h>

struct rir_fndef;

enum rir_obj_category {
    RIR_OBJ_EXPRESSION,
    RIR_OBJ_BLOCK,
    RIR_OBJ_TYPEDEF,
    RIR_OBJ_GLOBAL,
    RIR_OBJ_VARIABLE,
};

struct rir_object {
    enum rir_obj_category category;
    union {
        struct rir_expression expr;
        struct rir_block block;
        struct rir_typedef tdef;
        struct rir_global global;
        struct rir_variable variable;
    };
    //! Control to be added to the list of all rir objects
    struct RFilist_node ln;
};

struct rir_object *rir_object_create(enum rir_obj_category category, struct rir *r);
void rir_object_destroy(struct rir_object *obj);

struct rir_value *rir_object_value(struct rir_object *obj);

/**
 * Will remove a rir object from the global rir object list
 *
 * @param obj            The object to remove from the list
 * @param r              The rir module to remove from
 * @param current_fn     The current function during parsing/initialization
 */
void rir_object_listrem(struct rir_object *obj, struct rir *r, struct rir_fndef *current_fn);

/**
 * Will remove a rir object from the global rir object list and destroy the memory
 *
 * @param obj            The object to remove from the list
 * @param r              The rir module to remove from
 * @param current_fn     The current function during parsing/initialization
 */
void rir_object_listrem_destroy(struct rir_object *obj, struct rir *r, struct rir_fndef *current_fn);

/**
 * Will get the rir typedef from an object.
 *
 * @param obj        The object whose typedef to get
 * @return           If @a obj is a @a RIR_OBJ_TYPEDEF then the typedef is
 *                   directly returned. If it's a @a RIR_OBJ_VARIABLE then
 *                   if the type is a tdef that is returned. In other cases
 *                   NULL is returned.
 */
struct rir_typedef *rir_object_get_typedef(struct rir_object *obj);

/**
 * @return the object category in a human readable string
 */
const struct RFstring *rir_object_category_str(const struct rir_object *obj);

/**
 * @return a string describing the rir object
 */
const struct RFstring *rir_object_string(const struct rir_object *obj);

i_INLINE_DECL struct rir_expression *rir_object_to_expr(struct rir_object *obj)
{
    RF_ASSERT(obj->category == RIR_OBJ_EXPRESSION, "Expected expression object");
    return &obj->expr;
}
#endif
