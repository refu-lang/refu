rf_target_and_test_sources(refu test_refu_helper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/analyzer.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/analyzer_pass1.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/symbol_table.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/type_set.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/typecheck.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/typecheck_arr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/typecheck_functions.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/typecheck_forexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/typecheck_matchexpr.c")
