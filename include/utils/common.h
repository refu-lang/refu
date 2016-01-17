#ifndef LFR_UTILS_COMMON_H
#define LFR_UTILS_COMMON_H

/**
 * Denotes which code path we are following as far as rir is concerned.
 *
 * + RIRPOS_AST:
 * + RIRPOS_PARSE: Then the RIR is created by directly parsing the RIR source.
 */
enum rir_pos {
    //! Code path when the RIR is created normally by parsing the source file,
    //! generating the AST and then analyzing it to create the RIR.
    RIRPOS_AST = 1,
    //! Code path where RIR is created by directly parsing the RIR source.
    RIRPOS_PARSE = 2,
    //! Given when we don't want to specify code path, or we got illegal path.
    RIRPOS_NONE = 3,
};

#endif
