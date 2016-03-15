#ifndef LFR_UTILS_TRAVERSAL_H
#define LFR_UTILS_TRAVERSAL_H

#include <stdbool.h>
#include <rflib/defs/inline.h>

enum traversal_cb_res {
    //! Error occured in the callback
    TRAVERSAL_CB_ERROR = -2,
    //! Error occured in the callback, will stop traversal
    TRAVERSAL_CB_FATAL_ERROR = -1,
    //! Callback was succesfull
    TRAVERSAL_CB_OK = 0,
    //! Callback was succesfull and also stop iteration
    TRAVERSAL_CB_OK_AND_STOP = 1,
};

i_INLINE_DECL bool traversal_success(enum traversal_cb_res rc)
{
    return rc == TRAVERSAL_CB_OK || rc == TRAVERSAL_CB_OK_AND_STOP;
}

i_INLINE_DECL bool traversal_stop(enum traversal_cb_res rc)
{
    return rc == TRAVERSAL_CB_OK_AND_STOP || rc == TRAVERSAL_CB_FATAL_ERROR;
}

#endif
