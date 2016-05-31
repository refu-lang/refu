add_executable(test_refu "")

target_sources(test_refu PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/test_input_base.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/test_main.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/testsupport_front.c")

add_subdirectory(analyzer)
add_subdirectory(end_to_end)
add_subdirectory(lexer)
add_subdirectory(parser)
add_subdirectory(rir)
add_subdirectory(types)

# --- general stuff needed for tests
target_compile_definitions(test_refu PUBLIC RF_UNIT_TESTS)
find_package(Check REQUIRED)
target_link_libraries(test_refu PUBLIC check)
# --- stuff needed for refu tests
target_link_libraries(test_refu PUBLIC test_refu_helper)
rf_use_llvm(test_refu)

target_compile_definitions(test_refu PUBLIC "CLIB_TEST_HELPERS=\"${CMAKE_CURRENT_SOURCE_DIR}/../rfbase/test/test_helpers.h\"")
target_include_directories(test_refu PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
