target_sources(test_refu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_block.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_function.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_generics.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_ifexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_matchexpr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_misc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_modules.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_operators.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_typeclass.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_parser_typedesc.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport_parser.c")
