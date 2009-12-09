cmake_minimum_required(VERSION 2.6)

project(SPMFILTER)

# settings affecting build process
set(CMAKE_VERBOSE_MAKEFILE FALSE)
set(FLAG_DEBUG TRUE)

# ********* NO NEED TO CHANGE ANYTHING BELOW **********
#

# use our cmake moduels too
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# * * * find packages first * * *
find_package(Glib2 REQUIRED)
if(GLIB2_FOUND)
	include_directories(${GLIB2_INCLUDE_DIRS})
	link_directories(${GLIB2_LIBRARY_DIR})
endif(GLIB2_FOUND)

# find esmtp
find_package(Esmtp REQUIRED)
if(ESMTP_FOUND)
	include_directories(${ESMTP_INCLUDE_DIRS})
	link_directories(${ESMTP_LIBRARY_DIR})
endif(ESMTP_FOUND)

# check for header files
include(CheckIncludeFiles)
check_include_files(stdio.h HAVE_STDIO_H)
check_include_files(libesmtp.h HAVE_LIBESMTP_H)

# check for functions
include(CheckFunctionExists)
check_function_exists(strtoul HAVE_FUNC_STRTOUL)

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

add_subdirectory(src)

