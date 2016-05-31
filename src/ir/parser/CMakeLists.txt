include(CreateGperfHeader)
# generate hash table for the rir parser using gperf
create_gperf_header(${CMAKE_CURRENT_SOURCE_DIR} rirtoken_htable.gperf rirtoken_htable.h)

rf_target_and_test_sources(refu test_refu_helper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/rirparser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rirtoken.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_binaryop.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_blocks.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_expressions.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_functions.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_global.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_typedef.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_utils.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/rparse_value.c")

