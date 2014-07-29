#include <messaging.h>

#include <RFmemory.h>
#include <RFlocalmem.h>
#include <Utils/bits.h>
#include <Utils/sanity.h> //for sanity checks

/* message object code begins */

#define MSG_COORD_STR_BUFF 128
typedef struct message
{
    //! The actual message string
    struct RFstring s;
    //! Can attach the message to a list
    RFilist_node ln;
    //! The type of the message. Either warning or error for now.
    int type;
    //! The parsing context of the AST node that generated the message
    struct ast_context *ctx;
    struct RFstringx coord_string;
}message;

static bool message_init(message* m, struct RFstring *s,  message_type_t type,
                         struct ast_context *ctx)
{
#if 0
    RF_ASSERT(ctx->file_name);
#endif
    if(!rf_string_copy_in(&m->s, s))
    {
        return false;
    }
    m->type = type;
    m->ctx = ctx;
    if(!rf_stringx_init_buff(&m->coord_string, MSG_COORD_STR_BUFF, ""))
    {
        return false;
    }
    
    return true;
}

static message* message_create(struct RFstring *s, message_type_t type,
                               struct ast_context *ctx)
{
    message* ret;
    RF_MALLOC(ret, sizeof(message), NULL);
    if(!message_init(ret, s, type, ctx))
    {
        free(ret);
        ret = NULL;
    }
    return ret;
}

static void message_destroy(message* m)
{
    rf_string_deinit(&m->s);
    rf_stringx_deinit(&m->coord_string);
    free(m);
}

static struct RFstring *message_ctx_tostr(message* m)
{
    if(!rf_stringx_assign(&m->coord_string,
#if 0
                          RFS_("%S:%d",
                               &m->ctx->file_name, m->ctx->line_number)))
#else
        RFS_("PLACEHOLDER")))
#endif
    {
        print_error("Could not turn a context to a string");
        return NULL;
    }
    return &m->coord_string.INH_String;
}
/* message object code ends */





#define VERBOSE_LEVEL_MIN 0
#define VERBOSE_LEVEL_MAX 4

typedef struct messaging_context
{
    RFilist_head messages_list;
    int verbose_level;
}messaging_context;
static messaging_context _ctx;
static struct RFstringx _string_buff;






bool _add_message(struct RFstring *s, int type, struct ast_context *ctx)
{
    RF_ENTER_LOCAL_SCOPE();
    message* msg = message_create(s, type, ctx);
    if(!msg)
    {
        RF_EXIT_LOCAL_SCOPE();
        return false;
    }
    rf_ilist_add_tail(&_ctx.messages_list, &msg->ln);
    RF_EXIT_LOCAL_SCOPE();
    return true;
}


bool messaging_has_data()
{
    return !rf_ilist_is_empty(&_ctx.messages_list);
}


void flush_messages(FILE* f, int type)
{
    message* m, *tmp;
    rf_ilist_for_each_safe(&_ctx.messages_list, m, tmp, ln)
    {
        if(RF_BITFLAG_ON(type, MESSAGE_ANY) ||
           RF_BITFLAG_ON(type, m->type))
        {
            if(m->ctx)
            {
                switch(m->type)
                {
                    case MESSAGE_SEMANTIC_WARNING:
                        fprintf(
                            f,
                            "[Semantic Analysis Warning - "RF_STR_PF_FMT"]:"
                            RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(message_ctx_tostr(m)),
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_PARSING_WARNING:
                        fprintf(
                            f,
                            "[Parsing Warning - "RF_STR_PF_FMT"]:"
                            RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(message_ctx_tostr(m)),
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_SEMANTIC_ERROR:
                        fprintf(
                            f,
                            "[Semantic Analysis Error - "RF_STR_PF_FMT"]:"
                            RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(message_ctx_tostr(m)),
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_PARSING_ERROR:
                        fprintf(
                            f,
                            "[Parsing Error - "RF_STR_PF_FMT"]:"
                            RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(message_ctx_tostr(m)),
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    default: /* should never get here */
                        assert(0);
                        break;
                }
            }
            else
            {
                switch(m->type)
                {
                    case MESSAGE_SEMANTIC_WARNING:
                        fprintf(
                            f,
                            "[Semantic Analysis Warning]:"RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_PARSING_WARNING:
                        fprintf(
                            f,
                            "[Parsing Warning]:"RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_SEMANTIC_ERROR:
                        fprintf(
                            f,
                            "[Semantic Analysis Error]:"RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    case MESSAGE_PARSING_ERROR:
                        fprintf(
                            f,
                            "[Parsing Error]:"RF_STR_PF_FMT"\n",
                            RF_STR_PF_ARG(&m->s)
                        );
                        break;
                    default: /* should never get here */
                        assert(0);
                        break;
                }
            }
            rf_ilist_delete_from(&_ctx.messages_list, &m->ln);
            message_destroy(m);
        } 
    }
}


/* simple printing related functions */
void _print_error(struct RFstring* s)
{
    RF_ENTER_LOCAL_SCOPE();
    printf("refu: [ERROR] "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(s));
    RF_EXIT_LOCAL_SCOPE();
}

void _print_warning(struct RFstring* s)
{
    RF_ENTER_LOCAL_SCOPE();
    printf("refu: [WARNING] "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(s));
    RF_EXIT_LOCAL_SCOPE();
}

void _print_info_l(int level, struct RFstring* s)
{
    RF_ENTER_LOCAL_SCOPE();
    if(_ctx.verbose_level >= level)
    {
        printf("refu: [INFO] "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(s));
    }
    RF_EXIT_LOCAL_SCOPE();
}

void _print_info(struct RFstring* s)
{
    RF_ENTER_LOCAL_SCOPE();
    printf("refu: [INFO] "RF_STR_PF_FMT"\n", RF_STR_PF_ARG(s));
    RF_EXIT_LOCAL_SCOPE();
}


/* functions used for module initialization */
static inline bool check_string_value(char* s)
{
    if(!rf_stringx_assign(&_string_buff, RFS_(s)))
    {
        print_error("Could not assign input argument to string");
        return false;
    }
    if(!rf_string_to_int(&_string_buff, &_ctx.verbose_level))
    {
        print_error("Verbose level argument is not a number");
        return false;
    }
    return true;
}
static bool parse_args_verbose(int argc, char** argv)
{
    int i;
    _ctx.verbose_level = VERBOSE_LEVEL_DEFAULT;
    for(i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-v") == 0 ||
           strcmp(argv[i], "--verbose-level") == 0)
        {
            if(argc >= i)
            {
                return check_string_value(argv[i+1]);
            }
            print_error("A number should follow the verbose argument");
            return false;
        }

        if(strstr(argv[i], "--verbose-level="))
        {
            return check_string_value(argv[i]+16);
        }
    }
    return true;
}
bool messaging_modinit(int argc, char** argv)
{
    rf_ilist_head_init(&_ctx.messages_list);
    if(!rf_stringx_init_buff(&_string_buff, 128, ""))
    {
        return false;
    }


    if(!parse_args_verbose(argc, argv))
    {
        return false;
    }
    
    if(_ctx.verbose_level < VERBOSE_LEVEL_MIN ||
       _ctx.verbose_level > VERBOSE_LEVEL_MAX)
    {
        print_warning("Illegal value provided for verbosity level. "
                      "Must be between %d and %d. Defaulting to %d.",
                      VERBOSE_LEVEL_MIN, VERBOSE_LEVEL_MAX,
                      VERBOSE_LEVEL_DEFAULT);
        _ctx.verbose_level = VERBOSE_LEVEL_DEFAULT;
    }

    print_info_l(4, "Verbose level is: %d", _ctx.verbose_level);
    return true;
}
