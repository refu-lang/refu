#include <inpstr.h>

#include <rfbase/string/corex.h>
#include <rfbase/string/core.h>
#include <rfbase/utils/sanity.h>

bool inpstr_init(struct inpstr *s,
                 struct RFstringx *input_str,
                 struct RFarray *arr,
                 unsigned int lines_num)
{
    RF_STRINGX_SHALLOW_COPY(&s->str, input_str);
    s->lines_num = lines_num;
    RF_MALLOC(s->lines, sizeof(uint32_t) * lines_num, return false);
    memcpy(s->lines, arr->buff, sizeof(uint32_t) * lines_num);

    return true;
}

bool inpstr_init_from_source(struct inpstr *s,
                             const struct RFstring *input_str)
{
    static const struct RFstring nl = RF_STRING_STATIC_INIT("\n");
    struct RFarray arr;
    int lc;
    RF_ARRAY_TEMP_INIT(&arr, uint32_t, 128);
    if (!rf_stringx_from_string_in(&s->str, input_str)) {
        return false;
    }

    if ((lc = rf_string_count(input_str, &nl, 0, &arr, 0)) == -1) {
        return false;
    }
    s->lines_num = lc + 1;
    RF_MALLOC(s->lines, sizeof(uint32_t) * s->lines_num, return false);


    if (s->lines_num == 1) { //we got nothing to copy from, so don't
        s->lines[0] = 0;
    } else {
        unsigned i;
        s->lines[0] = 0;
        for (i = 1; i < s->lines_num; ++i) {
            s->lines[i] = rf_array_at_unsafe(&arr, i - 1, uint32_t) + 1;
        }
    }

    return true;
}

void inpstr_deinit(struct inpstr *s)
{
    rf_stringx_deinit(&s->str);
    free(s->lines);
}

bool inpstr_ptr_to_linecol(struct inpstr *s,
                           char *p, unsigned int *line,
                           unsigned int *col)
{
    uint32_t i;
    struct RFstring tmp;
    char *sp;
    bool found = false;
    char *sbeg = inpstr_beg(s);
    uint32_t off = p - sbeg;

    RF_ASSERT(s->lines_num > 0,
              "The input string line indexing should start from 1");

    for (i = 0; i < s->lines_num - 1; i++) {
        if (off >= s->lines[i] && off < s->lines[i + 1]) {
            found = true;
            break;
        }
    }
    if (!found) {
        if (off >= s->lines[s->lines_num - 1] &&
            off <= inpstr_len_from_beg(s)) {
            /* last line */
            i = s->lines_num - 1;
        } else {
            return false;
        }
    }

    *line = i;
    sp = sbeg + s->lines[*line];
    RF_ASSERT(p - sp >= 0,
              "pointer difference should always be positive");

    RF_STRING_SHALLOW_INIT(&tmp, sp, p - sp);
    *col = rf_string_length(&tmp);

    return true;
}

i_INLINE_INS struct RFstringx *inpstr_str(struct inpstr *s);
i_INLINE_INS char *inpstr_data(struct inpstr *s);
i_INLINE_INS char *inpstr_beg(const struct inpstr *s);
i_INLINE_INS uint32_t inpstr_len_from_beg(struct inpstr *s);
