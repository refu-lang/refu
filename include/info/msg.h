#ifndef LFR_INFO_MSG_H
#define LFR_INFO_MSG_H

#include <RFstring.h>
#include <RFintrusive_list.h>

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
    //! The location where the message was generated
    struct inplocation loc;
};

struct info_msg *info_msg_create(enum info_msg_type type,
                                 struct inplocation *loc,
                                 const char *fmt,
                                 va_list args);
void info_msg_destroy(struct info_msg *m);
void info_msg_print(struct info_msg *m, FILE *f, struct inpfile *input_file);

bool info_msg_get_formatted(struct info_msg *m, struct RFstringx *s,
                            struct inpfile *input_file);
#endif
