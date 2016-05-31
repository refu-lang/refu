# Find, use and configure LLVM for use with the given target
#
# TARGET -- The target for which to configure LLVM

function(rf_use_llvm TARGET)
  find_package(LLVM REQUIRED CONFIG)
  target_include_directories(${TARGET} PUBLIC ${LLVM_INCLUDE_DIRS})
  target_compile_definitions(${TARGET} PRIVATE ${LLVM_DEFINITIONS})
  llvm_map_components_to_libnames(llvm_libs core analysis executionengine interpreter native linker)
  target_link_libraries(${TARGET} PUBLIC ${llvm_libs})
endfunction()
