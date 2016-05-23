# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find GraphViz
# Find the native GraphViz headers and libraries.
#
# GRAPHVIZ_INCLUDE_DIRS	- where to find m_apm.h, etc.
# GRAPHVIZ_LIBRARIES	- List of libraries when using GraphViz.
# GRAPHVIZ_FOUND	- True if GraphViz found.

# Look for the header file.
FIND_PATH(GRAPHVIZ_INCLUDE_DIR NAMES graphviz/graph.h)

# Look for the library.
FIND_LIBRARY(GRAPHVIZ_gvc_LIBRARY NAMES gvc )
FIND_LIBRARY(GRAPHVIZ_graph_LIBRARY NAMES graph )
FIND_LIBRARY(GRAPHVIZ_cdt_LIBRARY NAMES cdt )
FIND_LIBRARY(GRAPHVIZ_pathplan_LIBRARY NAMES pathplan )

SET(GRAPHVIZ_LIBRARY ${GRAPHVIZ_gvc_LIBRARY} ${GRAPHVIZ_graph_LIBRARY} ${GRAPHVIZ_cdt_LIBRARY} ${GRAPHVIZ_pathplan_LIBRARY})

# Handle the QUIETLY and REQUIRED arguments and set GRAPHVIZ_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GRAPHVIZ DEFAULT_MSG GRAPHVIZ_LIBRARY GRAPHVIZ_INCLUDE_DIR)

# Copy the results to the output variables.
IF(GRAPHVIZ_FOUND)
	SET(GRAPHVIZ_LIBRARIES ${GRAPHVIZ_LIBRARY})
	SET(GRAPHVIZ_INCLUDE_DIRS ${GRAPHVIZ_INCLUDE_DIR})
ELSE(GRAPHVIZ_FOUND)
	SET(GRAPHVIZ_LIBRARIES)
	SET(GRAPHVIZ_INCLUDE_DIRS)
ENDIF(GRAPHVIZ_FOUND)

MARK_AS_ADVANCED(GRAPHVIZ_INCLUDE_DIRS GRAPHVIZ_LIBRARIES)
