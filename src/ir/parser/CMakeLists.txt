include(CreateGperfHeader)
# generate hash table for the rir parser using gperf
create_gperf_header(refu rirtoken_htable.gperf rirtoken_htable.h)

target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rirparser.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rirtoken.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_binaryop.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_blocks.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_expressions.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_functions.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_global.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_typedef.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_utils.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rparse_value.c")

