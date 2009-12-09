# find libesmtp
#

set(INCL_DIRS
	/usr/include
	/usr/local/include
	/opt/local/include
	/opt/gnome/include
)

set(LIB_DIRS
	/usr/lib
	/usr/local/lib
	/opt/local/lib
	/opt/gnome/lib
)

# find include dir
find_path(ESMTP_INCLUDE_DIR libesmtp.h
	PATHS ${INCL_DIRS}
)

# find lib
find_library(ESMTP_LIBRARY
	NAMES esmtp
	PATHS ${LIB_DIRS}
)

if(ESMTP_INCLUDE_DIR AND ESMTP_LIBRARY)
	set(ESMTP_FOUND TRUE)
	set(ESMTP_INCLUDE_DIRS ${ESMTP_INCLUDE_DIR})
	get_filename_component(ESMTP_LIBRARY_DIR ${ESMTP_LIBRARY} PATH)
endif(ESMTP_INCLUDE_DIR AND ESMTP_LIBRARY)

if(ESMTP_FOUND)
	if(NOT ESMTP_FIND_QUITELY)
		message(STATUS "Found esmtp library: ${ESMTP_LIBRARY}")
		message(STATUS "Found esmtp inc dirs: ${ESMTP_INCLUDE_DIRS}") 
	endif(NOT ESMTP_FIND_QUITELY)
else(ESMTP_FOUND)
	message(FATAL_ERROR "Could not find libesmtp")
endif(ESMTP_FOUND)

