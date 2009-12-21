#
# find glib-1.2 Library
#

INCLUDE(SetFindPaths)

# find include dir
find_path(GLIB_INCLUDE_DIR glib.h
	PATHS ${INCL_DIRS}
	PATH_SUFFIXES glib-1.2
)

# find glib config include
find_path(GLIB_CONFIG_INCLUDE_DIR glibconfig.h
	PATHS ${INCL_DIRS} ${LIB_DIRS}
	PATH_SUFFIXES /glib-1.2/include /glib/include
)

# find lib
find_library(GLIB_LIBRARY
	NAMES glib-1.2 glib-1.2.0 glib
	PATHS ${LIB_DIRS}
)

if(GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARY)
	set(GLIB_FOUND TRUE)
	set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR})
	list(APPEND GLIB_INCLUDE_DIRS ${GLIB_CONFIG_INCLUDE_DIR})
endif(GLIB_INCLUDE_DIR AND GLIB_CONFIG_INCLUDE_DIR AND GLIB_LIBRARY)

if(GLIB_FOUND)
	if(NOT GLIB_FIND_QUITELY)
		message(STATUS "Found glib-1.2 library: ${GLIB_LIBRARY}")
		message(STATUS "Found glib-1.2 inc dirs: ${GLIB_INCLUDE_DIRS}") 
	endif(NOT GLIB_FIND_QUITELY)
else(GLIB_FOUND)
	if(GLIB_FIND_REQUIRED)
		message(FATAL_ERROR "Could not find the glib-1.2 Library")
	else(GLIB_FIND_REQUIRED)
		message(STATUS "Could not find glib-1.2 Library")
	endif(GLIB_FIND_REQUIRED)
endif(GLIB_FOUND)

