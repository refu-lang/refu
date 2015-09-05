#ifndef LFR_IR_RIR_OBJECT_H
#define LFR_IR_RIR_OBJECT_H

#include <ir/rir_expression.h>
#include <ir/rir_argument.h>
#include <ir/rir_block.h>
#include <ir/rir_typedef.h>
#include <RFintrusive_list.h>

enum rir_obj_category {
    RIR_OBJ_EXPRESSION,
    RIR_OBJ_ARGUMENT,
    RIR_OBJ_BLOCK,
    RIR_OBJ_TYPEDEF,
};

struct rir_object {
    enum rir_obj_category category;
    union {
        struct rir_expression expr;
        struct rir_argument arg;
        struct rir_block block;
        struct rir_typedef tdef;
    };
    //! Control to be added to the list of all rir objects
    struct RFilist_node ln;
};

struct rir_object *rir_object_create(enum rir_obj_category category, struct rir *r);
void rir_object_destroy(struct rir_object *obj);
#endif
