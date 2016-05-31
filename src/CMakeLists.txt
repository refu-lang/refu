add_subdirectory(analyzer)
add_subdirectory(ast)
add_subdirectory(backend)
add_subdirectory(info)
add_subdirectory(ir)
add_subdirectory(lexer)
add_subdirectory(ownership)
add_subdirectory(parser)
add_subdirectory(serializer)
add_subdirectory(types)
add_subdirectory(utils)

include(RFTargetSources)
rf_target_and_test_sources(refu test_refu_helper PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/compiler.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler_args.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/front_ctx.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/inpfile.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/inplocation.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/inpoffset.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/inpstr.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/module.c")
target_sources(refu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
