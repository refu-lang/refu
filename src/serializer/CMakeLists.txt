target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/serializer.c")
if (${RF_OPTION_HAVE_JSONC})
  target_sources(refu PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/astprinter.c")
endif()
