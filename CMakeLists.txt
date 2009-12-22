cmake_minimum_required(VERSION 2.6)

project(spmfilter)

# settings affecting build process
set(CMAKE_VERBOSE_MAKEFILE TRUE)
set(FLAG_DEBUG TRUE)
set(SMF_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/spmfilter")

# internal project settings
set(SMF_PROJECT_VERSION "0.2")
set(SMF_ARCHIVE_NAME "${CMAKE_PROJECT_NAME}-${SMF_PROJECT_VERSION}")

# ********* NO NEED TO CHANGE ANYTHING BELOW **********
#
include(FindPkgConfig)

# use our cmake moduels too
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

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

# collect build info
include(HgBuildInfo)

# write config.h 
CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in 
	${CMAKE_CURRENT_SOURCE_DIR}/src/config.h
)

# check for enabled debugging
if(FLAG_DEBUG)
	add_definitions(-DDEBUG -g -O0)
endif(FLAG_DEBUG)

# build stuff in src directory
add_subdirectory(src)

# * * * custom targets * * *
add_custom_target(dist
	COMMAND hg archive --prefix="${SMF_ARCHIVE_NAME}" -t tgz "${SMF_ARCHIVE_NAME}.tar.gz"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
