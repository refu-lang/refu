# Call to create a gperf hashtable header from a gperf file
#
# Required Arguments:
#
# TARGET -- The target for which to call the gperf command
# INPUT -- The name of the input .gperf file
# OUTPUT -- The name of the output header file
#

function(create_gperf_header TARGET INPUT OUTPUT)
  add_custom_command(OUTPUT ${OUTPUT}
    PRE_BUILD
    COMMAND "${GPERF_EXECUTABLE} -t --output-file=${OUTPUT} ${INPUT}"
    COMMENT "Generating perfect hash table from ${INPUT}")
  add_custom_target(gperf_${INPUT} DEPENDS ${INPUT})
  add_dependencies(${TARGET} gperf_${INPUT})
endfunction()
