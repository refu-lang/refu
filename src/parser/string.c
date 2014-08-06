#include <parser/string.h>


bool parser_string_init(struct parser_string *s,
                        struct RFstringx *input_str,
                        struct RFarray *arr,
                        unsigned int lines_num)
{
    RF_STRINGX_SHALLOW_COPY(&s->str, input_str);
    s->lines_num = lines_num;
    RF_MALLOC(s->lines, sizeof(uint32_t) * lines_num, false);
    
    return true;
}

void parser_string_deinit(struct parser_string *s)
{
    rf_stringx_deinit(&s->str);
    free(s->lines);
}

i_INLINE_INS char *parser_string_data(struct parser_string *s);
