#
# find glib-2.0 Library
#

INCLUDE(SetFindPaths)

# find include dir
find_path(GLIB2_INCLUDE_DIR glib.h
	PATHS ${INCL_DIRS}
	PATH_SUFFIXES glib-2.0
)

# find glib config include
find_path(GLIB2_CONFIG_INCLUDE_DIR glibconfig.h
	PATHS ${INCL_DIRS} ${LIB_DIRS}
	PATH_SUFFIXES /glib-2.0/include 
)

# find lib
find_library(GLIB2_LIBRARY
	NAMES glib-2.0 
	PATHS ${LIB_DIRS}
)

if(GLIB2_INCLUDE_DIR AND GLIB2_CONFIG_INCLUDE_DIR AND GLIB2_LIBRARY)
	set(GLIB2_FOUND TRUE)
	set(GLIB2_INCLUDE_DIRS ${GLIB2_INCLUDE_DIR})
	get_filename_component(GLIB2_LIBRARY_DIR ${GLIB2_LIBRARY} PATH)
	list(APPEND GLIB2_INCLUDE_DIRS ${GLIB2_CONFIG_INCLUDE_DIR})
endif(GLIB2_INCLUDE_DIR AND GLIB2_CONFIG_INCLUDE_DIR AND GLIB2_LIBRARY)

if(GLIB2_FOUND)
	if(NOT GLIB2_FIND_QUITELY)
		message(STATUS "Found glib-2.0 library: ${GLIB2_LIBRARY}")
		message(STATUS "Found glib-2.0 dirs: ${GLIB2_INCLUDE_DIRS}") 
	endif(NOT GLIB2_FIND_QUITELY)
else(GLIB2_FOUND)
	if(GLIB2_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find the glib-2.0 Library")
	else(GLIB2_FIND_REQUIRED)
		message(STATUS "Could not find glib-2.0 Library")
	endif(GLIB2_FIND_REQUIRED)
endif(GLIB2_FOUND)

