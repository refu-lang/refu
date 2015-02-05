#include <analyzer/string_table.h>

#include <Utils/hash.h>
#include <Utils/log.h>
#include <Utils/memory.h>
#include <String/rf_str_core.h>

struct string_table_record {
    struct RFstring string;
    uint32_t hash;
};


static void string_table_record_destroy(void* record, void *user_arg)
{
    struct string_table_record *rec = record;
    rf_string_deinit(&rec->string);
    free(rec);
}

static size_t rehash_fn(const void *e, void *user_arg)
{
    const struct string_table_record *rec = e;
    return rf_hash_str_stable(&rec->string, 0);
}

static bool cmp_fn(const void *e, void *hash)
{
    const struct string_table_record *rec = e;
    return rec->hash == *(uint32_t*)hash;
}

bool string_table_init(struct string_table *t)
{
    htable_init(&t->table, rehash_fn, NULL);
    return true;
}

struct string_table *string_table_create()
{
    struct string_table *ret;
    RF_MALLOC(ret, sizeof(*ret), NULL);
    if (!string_table_init(ret)) {
        free(ret);
        ret = NULL;
    }
    return ret;
}

void string_table_deinit(struct string_table *t)
{
    // free memory of all strings
    htable_iterate_values(&t->table, string_table_record_destroy, NULL);
    htable_clear(&t->table);
}

void string_table_destroy(struct string_table *t)
{
    string_table_deinit(t);
    free(t);
}

bool string_table_add_str(struct string_table *t,
                          const struct RFstring *input,
                          uint32_t *out_hash)
{
    struct string_table_record *rec;
    uint32_t hash = rf_hash_str_stable(input, 0);

    // first see if it's already in the table
    rec = htable_get(&t->table, hash, cmp_fn, &hash);
    if (rec) {
        *out_hash = rec->hash;
        return true;
    }

    // create the string table record
    RF_MALLOC(rec, sizeof(*rec), return false);
    if (!rec) {
        RF_ERRNOMEM();
        return false;
    }
    if (!rf_string_copy_in(&rec->string, input)) {
        RF_ERRNOMEM();
        return false;
    }
    rec->hash = hash;

    // and add it
    if (!htable_add(&t->table, hash, rec)) {
        RF_ERROR("Could not add hash of "RF_STR_PF_FMT" to string table",
                 RF_STR_PF_ARG(input));
        string_table_record_destroy(rec, NULL);
        return false;
    }

    *out_hash = hash;
    return true;
}

const struct RFstring *string_table_get_str(const struct string_table *t,
                                            uint32_t hash)
{
    const struct string_table_record *rec;

    rec = htable_get(&t->table, hash, cmp_fn, &hash);
    if (!rec) {
        return NULL;
    }

    return &rec->string;
}

void string_table_iterate(struct string_table *t, string_table_cb cb, void* user_arg)
{
    htable_iterate_values(&t->table, (htable_iter_cb)cb, user_arg);
}
