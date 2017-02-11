# - Find gperf
# This module looks for gperf. This module defines the
# following values:
#  GPERF_EXECUTABLE: the full path to the gperf tool.
#  GPERF_FOUND: True if gperf has been found.

INCLUDE(FindCygwin)

function(rf_use_gperf TARGET)

  FIND_PROGRAM(GPERF_EXECUTABLE
    gperf
    ${CYGWIN_INSTALL_PATH}/bin
  )

  # handle the QUIETLY and REQUIRED arguments and set GPERF_FOUND to TRUE if
  # all listed variables are TRUE
  INCLUDE(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(Gperf DEFAULT_MSG GPERF_EXECUTABLE)

  # now run the gperf command and figure out the version
  execute_process(
    COMMAND ${GPERF_EXECUTABLE} "--version"
    OUTPUT_VARIABLE GPERF_VERSION_OUTPUT
  )

  string(REGEX MATCH
    "GNU gperf ([0-9])\\.([0-9])"
    GPERF_VERSION_STRING
    ${GPERF_VERSION_OUTPUT}
  )
  if(NOT CMAKE_MATCH_1 OR NOT CMAKE_MATCH_2)
    message(AUTHOR_WARNING "Error during check for gperf version.")
  endif()

  set(GPERF_MAJOR_VERSION ${CMAKE_MATCH_1})
  set(GPERF_MINOR_VERSION ${CMAKE_MATCH_2})

  target_compile_definitions(${TARGET} PUBLIC "GPERF_MAJOR_VERSION=${GPERF_MAJOR_VERSION}")
  target_compile_definitions(${TARGET} PUBLIC "GPERF_MINOR_VERSION=${GPERF_MINOR_VERSION}")

  MARK_AS_ADVANCED(GPERF_EXECUTABLE)
  MARK_AS_ADVANCED(GPERF_MAJOR_VERSION)
  MARK_AS_ADVANCED(GPERF_MINOR_VERSION)

  message("-- Found Gperf ${GPERF_MAJOR_VERSION}.${GPERF_MINOR_VERSION}")
endfunction()
