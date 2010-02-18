# find db4 library
#

find_library(HAVE_DB4 db)
if(HAVE_DB4-NOTFOUND)
	message(FATAL_ERROR "db4 has been enabled but library coud not be found")
else(HAVE_DB4-NOTFOUND)
	get_filename_component(DB4_PATH "${HAVE_DB4}" PATH)
	string(REGEX REPLACE "lib$" "include" DB4_INCLUDE_PATH ${DB4_PATH})
	message(STATUS "Found db4 library ${HAVE_DB4}")
	message(STATUS "Found db4 include path ${DB4_INCLUDE_PATH}")
endif(HAVE_DB4-NOTFOUND)
