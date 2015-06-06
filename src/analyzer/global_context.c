#include "global_context.h"

#include <ast/ast.h>
#include <analyzer/analyzer.h>
#include <types/type.h>
#include <types/type_elementary.h>

bool analyzer_load_globals(struct analyzer *a)
{
    // TODO: this will need refactoring when modules exist properly.
    //       then this should contain loading of some standard code
    return true;
}
