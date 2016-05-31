# Call to create a gperf hashtable header from a gperf file
#
# Required Arguments:
#
# DIR -- The directory in which the input and output files should be located
# INPUT -- The name of the input .gperf file
# OUTPUT -- The name of the output header file
#

function(create_gperf_header DIR INPUT OUTPUT)
  add_custom_command(OUTPUT ${DIR}/${OUTPUT}
    PRE_BUILD
    COMMAND ${GPERF_EXECUTABLE} -t --output-file=${DIR}/${OUTPUT} ${DIR}/${INPUT}
    COMMENT "Generating perfect hash table from ${DIR}/${INPUT}")
   add_custom_target(gperf_${INPUT} ALL DEPENDS ${DIR}/${OUTPUT})
endfunction()
