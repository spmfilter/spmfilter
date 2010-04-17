# find event library
#

find_library(HAVE_EVENT event)
if(HAVE_EVENT)
	get_filename_component(EVENT_PATH "${HAVE_EVENT}" PATH)
	string(REGEX REPLACE "lib$" "include" EVENT_INCLUDE_PATH ${EVENT_PATH})
	message(STATUS "  found event library ${HAVE_EVENT}")
	message(STATUS "  found event include path ${EVENT_INCLUDE_PATH}")
else(HAVE_LDAP)
	message(STATUS "  event library could not be found, disabling daemon mode.")
endif(HAVE_EVENT)
