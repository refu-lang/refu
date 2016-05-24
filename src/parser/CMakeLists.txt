add_subdirectory(recursive_descent)
target_sources(refu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/parser.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/parser_common.c")
