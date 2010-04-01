# find zdb database library
#

find_library(HAVE_ZDB zdb)
if(HAVE_ZDB)
	message(STATUS "  found zdb library ${HAVE_ZDB}")
	find_path(ZDB_HEADER_DIR NAMES 
		"zdb/URL.h" 
		"zdb/ResultSet.h" 
		"zdb/PreparedStatement.h" 
		"zdb/Connection.h" 
		"zdb/ConnectionPool.h" 
		"zdb/SQLException.h" 
		"zdb/Exception.h")
	string(REGEX REPLACE "include" "include/zdb" ZDB_INCLUDE_DIR ${ZDB_HEADER_DIR})
	message(STATUS "  found zdb include path ${ZDB_INCLUDE_DIR}")
else(HAVE_ZDB)
	message(STATUS "  zdb library could not be found, disabling sql backend.")
endif(HAVE_ZDB)
