#ifndef LFR_ANALYZER_STRING_TABLE_H
#define LFR_ANALYZER_STRING_TABLE_H

#include <Data_Structures/htable.h>
#include <Definitions/inline.h>

struct RFstring;

struct string_table {
    struct htable table;
};

bool string_table_init(struct string_table *t);
struct string_table *string_table_create();

void string_table_deinit(struct string_table *t);
void string_table_destroy(struct string_table *t);

/**
 * Allocates a string with the contents of @c input and stores it in the table.
 *
 * @param[in] t                 The string table in question
 * @param[in] input             The string to attempt to input
 * @param[out] out_hash         Returns the hash of the string.
 * @return                      True if all went okay. Also if the string is
 *                              already in the table. False if there was an error.
 */
bool string_table_add_str(struct string_table *t,
                          const struct RFstring *input,
                          uint32_t *out_hash);

/**
 * Retrieves a string with with a specific hash from a string table
 *
 * @param[in] t                 The string table in question
 * @param[in] hash              The hash whose string to retrieve
 * @return                      A pointer to the retrieved string or NULL if it
 *                              is not found.
 */
const struct RFstring *string_table_get_str(const struct string_table *t,
                                            uint32_t hash);

typedef void (*string_table_cb) (const struct RFstring *s, void *user_arg);
/**
 * Provide a callback to iterate the string table and do something for each string
 *
 * @param[in] t                 The string table in question
 * @paran[in] cb                The callback to be called for each string value
 * @param[in] user_arg          The user argument to provide to the callback
 */
void string_table_iterate(struct string_table *t, string_table_cb cb, void *user_arg);
#endif
