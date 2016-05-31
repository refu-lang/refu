# Add sources to multiple targets. Wrapper over target_sources() for multiple targets.
#
# Required Arguments:
#
# TARGETS -- A list of targets to add the sources to
# TYPE -- <INTERFACE|PUBLIC|PRIVATE> just as in the target_sources() function
#
# Additional Arguments:
# [source1, source2...] All source files to add to the target

function(rf_target_sources TARGETS TYPE)
  foreach(TARGET ${TARGETS})
    foreach(ARG_NUM RANGE 2 ${ARGC})
      target_sources(${TARGET} ${TYPE} ${ARGV${ARG_NUM}})
    endforeach()
  endforeach()
endfunction()


# Add sources to two targets, one is the normal target and the other is the test one
# iff the TESTS option is passed
# Required Arguments:
#
# TARGET -- The normal target
# TEST_TARGET -- The test target to add sources to if
# TYPE -- <INTERFACE|PUBLIC|PRIVATE> just as in the target_sources() function
#
# Additional Arguments:
# [source1, source2...] All source files to add to the target
function(rf_target_and_test_sources TARGET TEST_TARGET TYPE)
  foreach(ARG_NUM RANGE 3 ${ARGC})
    target_sources(${TARGET} ${TYPE} ${ARGV${ARG_NUM}})
    if (${TEST})
      target_sources(${TEST_TARGET} ${TYPE} ${ARGV${ARG_NUM}})
    endif()
  endforeach()
endfunction()
