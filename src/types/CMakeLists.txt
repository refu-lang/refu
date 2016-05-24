include(CreateGperfHeader)
# generate hash table for the elementary types using gperf
create_gperf_header(refu elementary_types_htable.gperf elementary_types_htable.h)

target_sources(refu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/type.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_arr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_comparisons.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_creation.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_elementary.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_function.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_operators.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_utils.c")
