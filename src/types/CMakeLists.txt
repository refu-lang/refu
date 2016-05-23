include(CreateGperfHeader)
# generate hash table for the elementary types using gperf
create_gperf_header(refu elementary_types_htable.gperf elementary_types_htable.h)

target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_arr.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_comparisons.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_creation.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_elementary.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_function.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_operators.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/type_utils.c")
