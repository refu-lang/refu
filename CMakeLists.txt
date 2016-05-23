cmake_minimum_required(VERSION 3.1.0)

# project name and version should be set after cmake_policy CMP0048
project(refu VERSION "0.5")
add_executable(refu "")
add_subdirectory(rfbase)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/rfbase/cmake/")
include(RFOption)

# --- Add refu options
rf_numerical_option(refu VERBOSE_LEVEL_DEFAULT
  "The default verbosity level. Should range between 1 and 4"
  1)
rf_numerical_option(refu INFO_CTX_BUFF_INITIAL_SIZE
  "The initial size in bytes of the info context buffer. This is the \
buffer used by the compiler to store all messages"
  512)
rf_numerical_option(refu INPUT_STRING_STARTING_LINES
"The initial number of lines for the line indexer of the buffered input file."
256)


rf_list_option(refu LANG_BACKEND
  "Specify the backend of the compiler to use"
  "LLVM" # for now only possible value is LLVM
  LLVM)

# try to find GraphViz and if requested, link to it
find_package(GraphViz)
if (${GRAPHVIZ_FOUND})
  rf_bool_option(refu WITH_GRAPHVIZ
    "If Graphviz is found in the system then compile with graphviz support."
    TRUE)

  if (${RF_OPTION_WITH_GRAPHVIZ})
    target_link_libraries(refu PUBLIC gvc cgraph cdt)
  endif()
endif()

# try to find libjson-c and link to it. It's used in ast printer
# since this is a work in progress and not really used yet it's optional
find_package(JSON-C)
if (${JSON-C_FOUND})
  rf_bool_option(refu HAVE_JSONC
    "If Json-c is found in the system then use it"
    TRUE)
  if (${RF_OPTION_HAVE_JSONC})
    target_link_libraries(refu PUBLIC ${JSON-C_LIBRARIES})
  endif()
endif()



find_package(Gperf REQUIRED)

# Deal with LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
target_include_directories(refu PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(refu PRIVATE ${LLVM_DEFINITIONS})
llvm_map_components_to_libnames(llvm_libs core analysis executionengine interpreter native linker)
target_link_libraries(refu PUBLIC ${llvm_libs})

add_subdirectory(src)
add_subdirectory(lib)

target_include_directories(refu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_include_directories(refu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(refu PUBLIC rfbase)
# Let the compiler know the root directory. Used for finding the location of the
# compiled librfbase when running the refu compiler itself
target_compile_definitions(refu PRIVATE "RF_LANG_CORE_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\"")


# Print compile definitions in case we want to debug
# could also have called rf_project_config(refu) just like we do for rfbase
get_target_property(REFU_COMPILE_DEFS refu COMPILE_DEFINITIONS)
MESSAGE("refu Compile Definitions: ${REFU_COMPILE_DEFS}")
