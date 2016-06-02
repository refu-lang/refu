# Find, use and configure LLVM for use with the given target
#
# TARGET -- The target for which to configure LLVM

function(rf_use_llvm TARGET)
  find_package(LLVM REQUIRED CONFIG)
  message("LLVM INCLUDES: ${LLVM_INCLUDE_DIRS}")
  target_include_directories(${TARGET} PUBLIC ${LLVM_INCLUDE_DIRS})
  message("LLVM DEFS: ${LLVM_DEFINITIONS}")
  target_compile_definitions(${TARGET} PUBLIC ${LLVM_DEFINITIONS})
  llvm_map_components_to_libnames(llvm_libs core analysis executionengine interpreter native linker)
  target_link_libraries(${TARGET} PUBLIC ${llvm_libs})
  message("LLVM LIBS: ${llvm_libs}")
endfunction()
