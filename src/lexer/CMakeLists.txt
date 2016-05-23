include(CreateGperfHeader)
# generate hash table for the parser using gperf
create_gperf_header(refu tokens_htable.gperf tokens_htable.h)

target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lexer.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tokens.c")
