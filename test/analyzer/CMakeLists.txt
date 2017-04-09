target_sources(test_refu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/test_modules.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_symbol_table.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_arr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_conversion.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_forexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_functions.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_matchexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_operators.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_typecheck_typeclasses.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport_analyzer.c")
