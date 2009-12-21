# find gmime library
#

INCLUDE(SetFindPaths)

SET(NAMES gmime-2.0 gmime-2.2 gmime-2.4)

# find include dir
find_path(GMIME_INCLUDE_DIR "gmime/gmime.h"
	PATHS ${INCL_DIRS}
	PATH_SUFFIXES ${NAMES}
)

# find lib
find_library(GMIME_LIBRARY
	NAMES ${NAMES}
	PATHS ${LIB_DIRS}
)

if(GMIME_INCLUDE_DIR AND GMIME_LIBRARY)
	set(GMIME_FOUND TRUE)
	set(GMIME_INCLUDE_DIRS ${GMIME_INCLUDE_DIR})
	get_filename_component(GMIME_LIBRARY_DIR ${GMIME_LIBRARY} PATH)
endif(GMIME_INCLUDE_DIR AND GMIME_LIBRARY)

if(GMIME_FOUND)
	if(NOT GMIME_FIND_QUITELY)
		message(STATUS "Found gmime library: ${GMIME_LIBRARY}")
		message(STATUS "Found gmime include: ${GMIME_INCLUDE_DIRS}") 
	endif(NOT GMIME_FIND_QUITELY)
else(GMIME_FOUND)
	message(FATAL_ERROR "Could not find gmime library")
endif(GMIME_FOUND)

