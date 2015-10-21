#ifndef LFR_IR_RIR_OBJECT_H
#define LFR_IR_RIR_OBJECT_H

#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir_block.h>
#include <ir/rir_typedef.h>
#include <ir/rir_global.h>
#include <ir/rir_variable.h>
#include <RFintrusive_list.h>

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
 * @param obj        The object to remove from the list
 * @param ctx        The rir context
 */
void rir_object_listrem(struct rir_object *obj, struct rir_ctx *ctx);

/**
 * Will remove a rir object from the global rir object list and destroy the memory
 *
 * @param obj        The object to remove from the list
 * @param ctx        The rir context
 */
void rir_object_listrem_destroy(struct rir_object *obj, struct rir_ctx *ctx);
#endif
