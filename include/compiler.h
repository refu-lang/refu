#ifndef LFR_REFU_COMPILER_H
#define LFR_REFU_COMPILER_H

#include <stdbool.h>

#include <rfbase/datastructs/intrusive_list.h>
#include <rfbase/string/core.h>

#include <utils/common.h>
#include <module.h>

struct compiler_args;
struct serializer;
struct rir;
struct info_ctx;

struct compiler {
    //! An error buffer for the compiler
    struct RFstringx err_buff;
    //! The object holding the compiler arguments
    struct compiler_args *args;
    //! A list of compiler's front end contexts. One for each file.
    struct RFilist_head front_ctxs;
    //! The serializer deals with data exporting and serialization (if needed)
    //! after the end of a succesfull analysis.
    struct serializer *serializer;
    //! Dynamic array to hold the memory of all created modules
    struct modules_arr modules;
    //! Sorted list of modules, after dependency resolution
    struct RFilist_head sorted_modules;
    //! Should stdlib be used or not? By default for now all main modules are using it
    bool use_stdlib;
    //! Pointer to the main front_ctxs
    struct front_ctx *main_front;
    struct RFstring llc_exec_path;
};

// a compiler will always be a unique singleton so we can get its instance
struct compiler *compiler_instance_get();
struct compiler *compiler_alloc();
bool compiler_init(struct compiler *c, int rf_logtype, bool with_stdlib);
struct compiler *compiler_create(int rf_logtype, bool with_stdlib);
struct compiler *compiler_create_with_args(int rf_logtype, bool with_stdlib, int argc, char **argv);
void compiler_destroy();

struct module *compiler_module_get(const struct RFstring *name);

/**
 * Create a new front context and add it to the compiler's front contexts
 *
 * @see front_ctx_create() for details
 */
struct front_ctx *compiler_new_front(
    struct compiler *c,
    enum rir_pos codepath,
    const struct RFstring *file_name,
    const struct RFstring *source
);

/**
 * Set a compiler's front as the main front_ctx
 */
bool compiler_set_main(struct front_ctx *c);

//! Passes arguments to the compiler and initializes the front end context
bool compiler_pass_args(int argc, char **argv);

bool compiler_preprocess_fronts();
bool compiler_analyze();
bool compiler_process();

//! Query compiler's argument and if help was requested, print help message and
//! return true. If true, program should exit succesfully
bool compiler_help_requested(struct compiler *c);


struct RFstringx *compiler_get_errors_from_front(struct compiler *c, struct info_ctx *info);
/**
 * Return a string with all the compiler errors/warning from all modules 
 */
struct RFstringx *compiler_get_errors(struct compiler *c);
void compiler_print_errors_from_front(struct compiler *c, struct info_ctx *info);
/**
 * Prints all the compiler errors/warning from all modules 
 */
void compiler_print_errors(struct compiler *c);

#endif
