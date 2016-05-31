# Call to create a gperf hashtable header from a gperf file
#
# Required Arguments:
#
# TARGET -- The target for which to call the gperf command
# INPUT -- The name of the input .gperf file
# OUTPUT -- The name of the output header file
#

include(RFOption)
include (RFUseLLVM)

function(refu_config TARGET)
  # --- Add refu options
  rf_numerical_option(${TARGET} VERBOSE_LEVEL_DEFAULT
    "The default verbosity level. Should range between 1 and 4"
    1)
  rf_numerical_option(${TARGET} INFO_CTX_BUFF_INITIAL_SIZE
    "The initial size in bytes of the info context buffer. This is the \
buffer used by the compiler to store all messages"
    512)
  rf_numerical_option(${TARGET} INPUT_STRING_STARTING_LINES
    "The initial number of lines for the line indexer of the buffered input file."
    256)

  rf_list_option(${TARGET} LANG_BACKEND
    "Specify the backend of the compiler to use"
    "LLVM" # for now only possible value is LLVM
    LLVM)

  # try to find GraphViz and if requested, link to it
  find_package(GraphViz)
  if (${GRAPHVIZ_FOUND})
    rf_bool_option(${TARGET} WITH_GRAPHVIZ
      "If Graphviz is found in the system then compile with graphviz support."
      TRUE)

    if (${RF_OPTION_WITH_GRAPHVIZ})
      target_link_libraries(${TARGET} PUBLIC gvc cgraph cdt)
    endif()
  endif()

  # try to find libjson-c and link to it. It's used in ast printer
  # since this is a work in progress and not really used yet it's optional
  find_package(JSON-C)
  if (${JSON-C_FOUND})
    rf_bool_option(${TARGET} HAVE_JSONC
      "If Json-c is found in the system then use it"
      TRUE)
    if (${RF_OPTION_HAVE_JSONC})
      target_link_libraries(${TARGET} PUBLIC ${JSON-C_LIBRARIES})
    endif()
  endif()



  find_package(Gperf REQUIRED)

  # Deal with LLVM
  find_package(LLVM REQUIRED CONFIG)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  rf_use_llvm(${TARGET})


  # link with rfbase
  target_link_libraries(${TARGET} PUBLIC rfbase)
  # Let the compiler know the root directory. Used for finding the location of the
  # compiled librfbase when running the refu compiler itself
  target_compile_definitions(${TARGET} PUBLIC "RF_LANG_CORE_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}\"")
  # add include directories
  target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
  target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endfunction()
