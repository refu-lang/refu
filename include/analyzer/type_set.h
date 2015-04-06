#ifndef LFR_TYPES_SET_H
#define LFR_TYPES_SET_H

#include <Data_Structures/objset.h>

struct type;
// Simply defining a set of type pointers for convenience
struct type_set {OBJSET_MEMBERS(struct type*);};

#endif
