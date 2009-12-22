cmake_minimum_required(VERSION 2.6)

project(spmfilter)

# check for build.properties
include("${CMAKE_SOURCE_DIR}/build.properties" OPTIONAL)

# check prefix
if(PREFIX)
	SET(CMAKE_INSTALL_PREFIX ${PREFIX})
endif(PREFIX)

# internal project settings
set(SMF_PROJECT_VERSION "0.2")
set(SMF_ARCHIVE_NAME "${CMAKE_PROJECT_NAME}-${SMF_PROJECT_VERSION}")
set(SMF_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/spmfilter")
message(STATUS "using smf library dir ${SMF_LIB_DIR}")

# ********* NO NEED TO CHANGE ANYTHING BELOW **********
# use our cmake moduels too
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# includes
include(FindPkgConfig)
include(HgBuildInfo)

# * * * find packages first * * *
find_package(Esmtp REQUIRED)
if(ESMTP_FOUND)
	include_directories(${ESMTP_INCLUDE_DIRS})
	link_directories(${ESMTP_LIBRARY_DIR})
endif(ESMTP_FOUND)

pkg_search_module(GLIB2 REQUIRED glib-2.0)
if(GLIB2_FOUND)
	include_directories(${GLIB2_INCLUDE_DIRS})
	link_directories(${GLIB2_LIBRARY_DIRS})
endif(GLIB2_FOUND)

pkg_search_module(GMIME REQUIRED gmime-2.4 gmime-2.2)
if(GMIME_FOUND)
	include_directories(${GMIME_INCLUDE_DIRS})
	link_directories(${GMIME_LIBRARY_DIRS})
	IF(${GMIME_VERSION} MATCHES "^2.4*")
		set(HAVE_GMIME24 TRUE)
	ENDIF(${GMIME_VERSION} MATCHES "^2.4*")
endif(GMIME_FOUND)

if(WITH_ZDB)
	find_package(Zdb REQUIRED)
	include_directories(${ZDB_INCLUDE_PATH})
endif(WITH_ZDB)

# check if we need to enable ldap
if(WITH_LDAP)
	find_package(Ldap REQUIRED)
	include_directories(${LDAP_INCLUDE_DIR})
endif(WITH_LDAP)


# write spmfilter.h
CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter.h.in 
	${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter.h
)

# check for enabled debugging
if(ENABLE_DEBUG)
	set(CMAKE_VERBOSE_MAKEFILE TRUE)
	add_definitions(-DDEBUG -g -O0)
endif(ENABLE_DEBUG)

# build stuff in src directory
add_subdirectory(src)
add_subdirectory(man)

# * * * custom targets * * *
add_custom_target(dist
	COMMAND hg archive --prefix="${SMF_ARCHIVE_NAME}" -t tgz "${SMF_ARCHIVE_NAME}.tar.gz"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

