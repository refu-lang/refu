#ifndef LFR_UTILS_DATA_H
#define LFR_UTILS_DATA_H

#include <stdbool.h>

/**
 * Returns the name of the location where the compiler can save data in the system
 *
 * @TODO Make this customizable
 */
const struct RFstring *rf_data_dir();

/**
 * Ensure the data directory exists and if it does not already then create it
 */
bool rf_data_dir_exists();

#endif
