#include <ir/rir_global.h>
#include <ir/rir.h>
#include <ir/rir_object.h>
#include <ir/parser/rirparser.h>

#include <Utils/hash.h>
#include <String/rf_str_core.h>
#include <String/rf_str_common.h>
#include <String/rf_str_manipulationx.h>

#include <utils/common_strings.h>

#include <ast/ast.h>
#include <ast/identifier.h>
#include <ast/string_literal.h>

static bool rir_global_init_string(struct rir_object *obj,
                                   struct rir_type *type,
                                   const struct RFstring *name,
                                   const void *value,
                                   struct rirobj_strmap *global_rir_map)
{
    // only elementary types for now
    if (!rir_type_is_elementary(type)) {
        RF_ERROR("Tried to create a global non elementary type");
        return false;
    }

    struct rir_global *global = &obj->global;
    if (type->etype != ELEMENTARY_TYPE_STRING) {
        RF_ERROR("We only support string literal globals for now");
        return false;
    }
    RFS_PUSH();
    bool ret = rir_value_literal_init(
        &global->val,
        obj,
        RFS(RF_STR_PF_FMT, RF_STR_PF_ARG(name)),
        value,
        global_rir_map
    );
    RFS_POP();
    return ret;
}

struct rir_object *rir_global_create_string(struct rir_type *type,
                                            const struct RFstring *name,
                                            const void *value,
                                            struct rir *rir)
{
    struct rir_object *ret = rir_object_create(RIR_OBJ_GLOBAL, rir);
    if (!ret) {
        return NULL;
    }
    if (!rir_global_init_string(ret, type, name, value, &rir->map)) {
        goto fail_free_ret;
    }
    // now also add the key to the global literals map
    if (!strmap_add(&rir->global_literals, &rir_object_value(ret)->literal, ret)) {
        RF_ERROR("Failed to add a string literal to the global string map");
        goto fail_free_ret;
    }
    return ret;

fail_free_ret:
    free(ret);
    return NULL;
}

struct rir_object *rir_global_create_parsed(struct rir_parser *p,
                                            const struct RFstring *name,
                                            const struct ast_node *type,
                                            const struct ast_node *value)
{
    if (!rf_string_equal(ast_identifier_str(type), &g_str_string)) {
        rirparser_synerr(p, ast_node_startmark(type), NULL,
                         "For now only global strings can be declared.");
        return NULL;
    }
    const struct RFstring *valstr = ast_string_literal_get_str(value);
    // take the gstr_token which should be 7 tokens behind since we are at ')'.
    struct token *gstr_tok = lexer_lookback(&p->lexer, 7);
    RFS_PUSH();
    struct rir_object *ret = NULL;
    const struct RFstring *expstr = RFS("$gstr_%u", rf_hash_str_stable(valstr, 0));
    if (!rf_string_equal(name, expstr)) {
        rirparser_synerr(
            p,
            token_get_start(gstr_tok),
            token_get_end(gstr_tok),
            "Mismatch in the name of a global identifier string "
            "with its value. Expectd " RF_STR_PF_FMT " but got "
            RF_STR_PF_FMT".",
            RF_STR_PF_ARG(expstr),
            RF_STR_PF_ARG(name)
        );
        goto end;
    }

    ret = rir_global_create_string(
        rir_type_elem_create(ELEMENTARY_TYPE_STRING, false),
        name,
        valstr,
        rir_parser_rir(p)
    );


end:
    RFS_POP();
    return ret;
}

void rir_global_deinit(struct rir_global *global)
{
    rir_value_deinit(&global->val);
}

bool rir_global_tostring(struct rirtostr_ctx *ctx, const struct rir_global *g)
{
    bool ret;
    RFS_PUSH();
    ret = rf_stringx_append(
        ctx->rir->buff,
        RFS(RF_STR_PF_FMT" = global("RF_STR_PF_FMT", \""RF_STR_PF_FMT"\")\n",
            RF_STR_PF_ARG(rir_global_name(g)),
            RF_STR_PF_ARG(rir_type_string(rir_global_type(g))),
            RF_STR_PF_ARG(rir_value_actual_string(&g->val)))
    );
    RFS_POP();
    return ret;
}


i_INLINE_INS const struct RFstring *rir_global_name(const struct rir_global *g);
i_INLINE_INS struct rir_type *rir_global_type(const struct rir_global *g);

struct rir_object *rir_global_addorget_string(struct rir *rir, const struct RFstring *s)
{
    struct rir_object *gstring = strmap_get(&rir->global_literals, s);
    if (!gstring) {
        RFS_PUSH();
        gstring = rir_global_create_string(
            rir_type_elem_create(ELEMENTARY_TYPE_STRING, false),
            RFS("$gstr_%u", rf_hash_str_stable(s, 0)),
            s,
            rir
        );
        RFS_POP();
    }
    return gstring;
}
