# find zdb database library
#

find_library(HAVE_ZDB zdb)
if(HAVE_ZDB-NOTFOUND)
	message(FATAL_ERROR "zdb has been enabled but library coud not be found")
else(HAVE_ZDB-NOTFOUND)
	get_filename_component(ZDB_PATH "${HAVE_ZDB}" PATH)
	string(REGEX REPLACE "lib$" "include/zdb" ZDB_INCLUDE_PATH ${ZDB_PATH})
	message(STATUS "Found zdb library ${HAVE_ZDB}")
	message(STATUS "Found zdb include path ${ZDB_INCLUDE_PATH}")
endif(HAVE_ZDB-NOTFOUND)

