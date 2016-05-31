cmake_minimum_required(VERSION 3.1.0)

# project name and version should be set after cmake_policy CMP0048
project(refu VERSION "0.5")
add_executable(refu "")
add_subdirectory(rfbase)
if (${TEST})
  add_library(test_refu_helper "")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/rfbase/cmake/")
include(RFOption)

include(RFTargetSources)
include(RFRefuConfig)
include(RFProjectConfig)
refu_config(refu)
rf_project_config(refu)
if (${TEST})
  refu_config(test_refu_helper)
  rf_project_config(test_refu_helper)
endif()

add_subdirectory(src)
add_subdirectory(lib)


if (${TEST})
  add_subdirectory(test)
endif()

# Print compile definitions in case we want to debug
# could also have called rf_project_config(refu) just like we do for rfbase
get_target_property(REFU_COMPILE_DEFS refu COMPILE_DEFINITIONS)
MESSAGE("refu Compile Definitions: ${REFU_COMPILE_DEFS}")


