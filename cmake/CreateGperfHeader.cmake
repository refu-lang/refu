# Call to create a gperf hashtable header from a gperf file
#
# Required Arguments:
#
# DIR -- The directory in which the input and output files should be located
# INPUT -- The name of the input .gperf file
# OUTPUT -- The name of the output header file
#

function(create_gperf_header DIR INPUT OUTPUT)
  # Add a fake custom target so that gperf creation always runs
  add_custom_target(
    gperf_${INPUT}_always_run ALL
    DEPENDS ${DIR}/fake.h)

  add_custom_command(
    OUTPUT
        ${DIR}/fake.h # This is the fake
        ${DIR}/${OUTPUT}
    PRE_BUILD
    COMMAND ${GPERF_EXECUTABLE} -t --output-file=${DIR}/${OUTPUT} ${DIR}/${INPUT}
    COMMENT "Generating perfect hash table from ${DIR}/${INPUT}")
endfunction()
