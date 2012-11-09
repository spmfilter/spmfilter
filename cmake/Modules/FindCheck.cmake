find_library(CHECK_LIB check)
find_path(CHECK_INCLUDES check.h)

if(CHECK_LIB)
	message(STATUS "  found check library ${CHECK_LIB}")
else(CHECK_LIB)
	MESSAGE(FATAL_ERROR "Unable to locate the check-library") 
endif(CHECK_LIB)

if(CHECK_INCLUDES)
	message(STATUS "  found check includes in ${CHECK_INCLUDES}")
else(CHECK_INCLUDES)
	MESSAGE(FATAL_ERROR "Unable to locate the check include path") 
endif(CHECK_INCLUDES)
