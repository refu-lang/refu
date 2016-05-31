add_subdirectory(creation)

target_sources(test_refu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/test_finalized_ast.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_ownership.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parsing_rir.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_rir_end_to_end.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_rir_misc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport_rir.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport_rir_compare.c")
