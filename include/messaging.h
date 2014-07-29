#ifndef MESSAGING_H
#define MESSAGING_H

#include <stdbool.h>
#include <RFstring.h>
#include <RFintrusive_list.h>

#if 0 
#include <ast/common.h> //include ast/context.h which is needed and also
                        //will include ast_common_get_context() for those who need it
#else
struct ast_context;
#endif


typedef enum{
    MESSAGE_ANY = 0x1,
    MESSAGE_SEMANTIC_WARNING = 0x2,
    MESSAGE_PARSING_WARNING = 0x4,
    MESSAGE_SEMANTIC_ERROR = 0x8,
    MESSAGE_PARSING_ERROR = 0x10
}message_type_t;



bool messaging_modinit(int argc, char** argv);

#define add_parsing_error(ctx_, ...) \
    _add_message(RFS_(__VA_ARGS__), MESSAGE_PARSING_ERROR, ctx_)
#define add_parsing_warning(ctx_, ...) \
    _add_message(RFS_(__VA_ARGS__), MESSAGE_PARSING_WARNING, ctx_)
#define add_semantic_error(ctx_, ...) \
    _add_message(RFS_(__VA_ARGS__), MESSAGE_SEMANTIC_ERROR, ctx_)
#define add_semantic_warning(ctx_, ...) \
    _add_message(RFS_(__VA_ARGS__), MESSAGE_SEMANTIC_WARNING, ctx_)
bool _add_message(struct RFstring *s, int type, struct ast_context *ctx);


bool messaging_has_data();

/**
 * Flush all messages of one or more types from the messaging
 * context into a stream
 * @param f The stream to write to
 * @param type A bit flag of message types. Types can be combined 
 * using logical OR. Legal values are those in the @ref message_type_t
 * enum
 */
void flush_messages(FILE* f, int type);



#ifdef DEBUG_FLEX
#define FDEBUG(...)  \
    RF_DEBUG(__VA_ARGS__)
#else
#define FDEBUG(...)  
#endif

#ifdef DEBUG_BISON
#define BDEBUG(...)  \                           \
    RF_DEBUG(__VA_ARGS__)
#else
#define BDEBUG(...)  
#endif


/**
 * Prints a generic non-compiler related error
 */
#define print_error(...) _print_error(RFS_(__VA_ARGS__))
void _print_error(struct RFstring *s);

/**
 * Prints an info string depending on the verbosity level
 * @param level The verbose level above which to print the message
 */
#define print_info_l(_l, ...) _print_info_l(_l, RFS_(__VA_ARGS__))
void _print_info_l(int level, struct RFstring *s);

/**
 * Prints an info string
 */
#define print_info(...) _print_info(RFS_(__VA_ARGS__))
void _print_info(struct RFstring *s);

/**
 * Prints a warning string
 */
#define print_warning(...) _print_warning(RFS_(__VA_ARGS__))
void _print_warning(struct RFstring *s);


#endif //include guards end
