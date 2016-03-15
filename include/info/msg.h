#ifndef LFR_INFO_MSG_H
#define LFR_INFO_MSG_H

#include <rflib/string.h>
#include <rflib/datastructs/intrusive_list.h>

#include <inplocation.h>
#include <info/info.h>

#define MSG_COORD_STR_BUFF 128

struct info_msg {
    //! The actual message string
    struct RFstring s;
    //! Can attach the message to a list
    RFilist_node ln;
    //! The type of the message. Either warning or error for now.
    enum info_msg_type type;
    //! The start mark to put for the error
    struct inplocation_mark start_mark;
    //! The end mark to put for the error
    struct inplocation_mark end_mark;
};

i_INLINE_DECL bool info_msg_has_end_mark(struct info_msg *msg)
{
    return msg->end_mark.p != NULL;
}

struct info_msg *info_msg_create(enum info_msg_type type,
                                 const struct inplocation_mark *start,
                                 const struct inplocation_mark *end,
                                 const char *fmt,
                                 va_list args);
void info_msg_destroy(struct info_msg *m);
void info_msg_print(struct info_msg *m, FILE *f, struct inpfile *input_file);

bool info_msg_get_formatted(struct info_msg *m, struct RFstringx *s,
                            struct inpfile *input_file);

const struct RFstring *info_msg_type_to_str(enum info_msg_type type);
#endif
