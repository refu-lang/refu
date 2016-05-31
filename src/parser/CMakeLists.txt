add_subdirectory(recursive_descent)
rf_target_and_test_sources(refu test_refu_helper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser_common.c")
