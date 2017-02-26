#include <types/type.h>

#include <rfbase/persistent/buffers.h>
#include <rfbase/utils/bits.h>
#include <rfbase/defs/threadspecific.h>
#include <rfbase/string/core.h>

#include <module.h>
#include <analyzer/analyzer.h>
#include <analyzer/typecheck.h>
#include <ast/ast.h>
#include <ast/type.h>
#include <ast/generics.h>
#include <ast/vardecl.h>
#include <ast/function.h>
#include <ast/identifier.h>

#include <utils/common_strings.h>

#include <types/type_operators.h>
#include <types/type_elementary.h>
#include <types/type_function.h>
#include <types/type_comparisons.h>
#include <types/type_arr.h>

static struct RFstring *type_str_do(const struct type *t, int options)
{
    struct RFstring *ret = RFS("");
    switch(t->category) {
    case TYPE_CATEGORY_ELEMENTARY:
        ret = RFS(
            RFS_PF,
            RFS_PA((struct RFstring*)type_elementary_get_str(t->elementary.etype))
        );
        break;
    case TYPE_CATEGORY_OPERATOR:
    {
        size_t sz = 0;
        struct type **subt;
        darray_foreach(subt, t->operator.operands) {
            ret = RFS(RFS_PF RFS_PF, RFS_PA(ret), RFS_PA(type_str_do(*subt, options)));
            if (darray_size(t->operator.operands) - 1 != sz) {
                ret = RFS(RFS_PF RFS_PF, RFS_PA(ret), RFS_PA(type_op_str(t->operator.type)));
            }
            ++sz;
        }
    }
    break;
    case TYPE_CATEGORY_DEFINED:
        ret = RFS(RFS_PF, RFS_PA(t->defined.name));
        break;
    case TYPE_CATEGORY_WILDCARD:
        ret = RFS(RFS_PF, RFS_PA(&g_str_wildcard));
        break;
    case TYPE_CATEGORY_ARRAY:
        ret = type_str_add_array(
            type_str_or_die(t->array.member_type, TSTR_DEFAULT),
            &t->array.dimensions
        );
        break;
    default:
        RF_CRITICAL_FAIL("TODO: Not yet implemented");
        return NULL;
    }

    return ret;
}

struct RFstring *type_str(const struct type *t, int options)
{
    if (t->category == TYPE_CATEGORY_DEFINED && options == TSTR_DEFINED_WITH_CONTENTS) {
        struct RFstring *sdefined = type_str_do(t->defined.type, TSTR_DEFAULT);
        if (!sdefined) {
            return NULL;
        }
        return RFS(RFS_PF" {" RFS_PF "}",
                   RFS_PA(t->defined.name),
                   RFS_PA(sdefined));
    } else if (t->category == TYPE_CATEGORY_DEFINED && options == TSTR_DEFINED_ONLY_CONTENTS) {
        struct RFstring *sdefined = type_str_do(t->defined.type, TSTR_DEFAULT);
        if (!sdefined) {
            return NULL;
        }
        return RFS(RFS_PF, RFS_PA(sdefined));
    } else {
        return type_str_do(t, options);
    }
}
i_INLINE_INS struct RFstring *type_str_or_die(const struct type *t, int options);

struct RFstring *type_op_create_str(const struct type *t1,
                                    const struct type *t2,
                                    enum typeop_type optype)
{
    return RFS(
        RFS_PF RFS_PF RFS_PF,
        RFS_PA(type_str_or_die(t1, TSTR_DEFAULT)),
        RFS_PA(type_op_str(optype)),
        RFS_PA(type_str_or_die(t2, TSTR_DEFAULT))
    );
}

size_t type_get_uid(const struct type *t)
{
    size_t ret;
    RFS_PUSH();
    ret = rf_hash_str_stable(type_str_or_die(t, TSTR_DEFINED_WITH_CONTENTS), 0);
    RFS_POP();
    return ret;
}

const struct RFstring *type_get_unique_type_str(const struct type *t)
{
    if (t->category == TYPE_CATEGORY_DEFINED) {
        t = t->defined.type;
    } else if (t->category == TYPE_CATEGORY_ELEMENTARY) {
        return type_elementary_get_str(t->elementary.etype);
    }
    return RFS_OR_DIE("internal_struct_%u", type_get_uid(t));
}

static struct type g_wildcard_type = {.category = TYPE_CATEGORY_WILDCARD};
const struct type *type_get_wildcard()
{
    return &g_wildcard_type;
}

i_INLINE_INS bool type_is_defined(const struct type *t);
i_INLINE_INS const struct RFstring *type_defined_get_name(const struct type *t);
i_INLINE_INS struct type *type_defined_get_type(const struct type *t);
