#include <analyzer/string_table.h>

#include <Utils/hash.h>
#include <Utils/log.h>
#include <String/rf_str_core.h>

static void string_table_string_destroy(void* string, void *user_arg)
{
    rf_string_destroy(string);
}

static size_t rehash_fn(const void *e, void *user_arg)
{
    return rf_hash_str_stable(e, 0);
}

static bool cmp_fn(const void *e, void *other_str)
{
    return rf_string_equal(e, other_str);
}

bool string_table_init(struct string_table *t)
{
    htable_init(&t->table, rehash_fn, NULL);
    return true;
}

void string_table_deinit(struct string_table *t)
{
    // free memory of all strings
    htable_iterate_values(&t->table, string_table_string_destroy, NULL);
    htable_clear(&t->table);
}

bool string_table_add_str(struct string_table *t,
                          struct RFstring *input,
                          uint32_t *out_hash)
{
    struct RFstring *str;
    uint32_t hash = rf_hash_str_stable(input, 0);

    // first see if it's already in the table
    str = htable_get(&t->table, hash, cmp_fn, input);
    if (str) {
        *out_hash = hash;
        return true;
    }

    str = rf_string_copy_out(input);
    if (!str) {
        RF_ERRNOMEM();
        return false;
    }

    if (!htable_add(&t->table, hash, str)) {
        rf_string_destroy(str);
        return false;
    }

    *out_hash = hash;
    return true;
}


