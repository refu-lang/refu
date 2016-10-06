include(CreateGperfHeader)
# generate hash table for the parser using gperf
create_gperf_header(${CMAKE_CURRENT_SOURCE_DIR} tokens_htable.gperf tokens_htable.h)

rf_target_and_test_sources(refu test_refu_helper PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/lexer.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/tokens.c")
