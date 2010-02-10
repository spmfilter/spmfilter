# find zdb database library
#

find_library(HAVE_ZDB zdb)
if(HAVE_ZDB-NOTFOUND)
	message(FATAL_ERROR "zdb has been enabled but library coud not be found")
else(HAVE_ZDB-NOTFOUND)
	message(STATUS "Found zdb library ${HAVE_ZDB}")
	find_path(ZDB_HEADER_DIR NAMES 
		"zdb/URL.h" 
		"zdb/ResultSet.h" 
		"zdb/PreparedStatement.h" 
		"zdb/Connection.h" 
		"zdb/ConnectionPool.h" 
		"zdb/SQLException.h" 
		"zdb/Exception.h")
	string(REGEX REPLACE "include" "include/zdb" ZDB_INCLUDE_DIR ${ZDB_HEADER_DIR})
	message(STATUS "Found zdb include path ${ZDB_INCLUDE_DIR}")
endif(HAVE_ZDB-NOTFOUND)
