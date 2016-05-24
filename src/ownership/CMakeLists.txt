target_sources(refu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ownership.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ow_edge.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ow_graph.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/ow_node.c")
if (${RF_OPTION_WITH_GRAPHVIZ})
  target_sources(refu PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ow_graphviz.c")
endif()
