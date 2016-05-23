add_subdirectory(recursive_descent)
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/parser.c")
target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/parser_common.c")
