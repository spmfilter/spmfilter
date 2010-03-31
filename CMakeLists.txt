cmake_minimum_required(VERSION 2.6)

project(spmfilter)

# check for build.properties
include("${CMAKE_SOURCE_DIR}/build.properties" OPTIONAL)

# check prefix
if(PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${PREFIX})
endif(PREFIX)

# internal project settings
set(SMF_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib/spmfilter")
message(STATUS "using smf library dir ${SMF_LIB_DIR}")

# ********* NO NEED TO CHANGE ANYTHING BELOW **********
# use our cmake moduels too
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# includes
include(FindPkgConfig)
include(SMFMacros)

# * * * find packages first * * *
find_package(Esmtp REQUIRED)
if(ESMTP_FOUND)
    include_directories(${ESMTP_INCLUDE_DIRS})
    link_directories(${ESMTP_LIBRARY_DIR})
endif(ESMTP_FOUND)

pkg_search_module(GLIB214 glib-2.0>=2.14)
if(GLIB214_FOUND)
    include_directories(${GLIB214_INCLUDE_DIRS})
    link_directories(${GLIB214_LIBRARY_DIRS})
	set(HAVE_GLIB214 TRUE)
else(GLIB214_FOUND)
	pkg_check_modules(GLIB2 REQUIRED glib-2.0<=2.14 libpcre)
	if(GLIB2_FOUND)
		include_directories(${GLIB2_INCLUDE_DIRS})
		link_directories(${GLIB2_LIBRARY_DIRS})
		set(HAVE_PCRE TRUE)
	endif(GLIB2_FOUND)
endif(GLIB214_FOUND)

pkg_search_module(GMODULE REQUIRED gmodule-2.0)
if(GMODULE_FOUND)
    include_directories(${GMODULE_INCLUDE_DIRS})
    link_directories(${GMODULE_LIBRARY_DIRS})
endif(GMODULE_FOUND)

pkg_search_module(GMIME REQUIRED gmime-2.4 gmime-2.0)
if(GMIME_FOUND)
    include_directories(${GMIME_INCLUDE_DIRS})
    link_directories(${GMIME_LIBRARY_DIRS})
	set(GMIME_VERSION_REGEX "^2\\.4\\.[0-9]+")
	IF(${GMIME_VERSION} MATCHES ${GMIME_VERSION_REGEX})
		set(HAVE_GMIME24 TRUE)
    ENDIF(${GMIME_VERSION} MATCHES ${GMIME_VERSION_REGEX})
endif(GMIME_FOUND)

if(WITH_ZDB)
    find_package(Zdb REQUIRED)
    include_directories(${ZDB_INCLUDE_DIR})
	link_directories(${ZDB_PATH})
endif(WITH_ZDB)

# check if we need to enable ldap
if(WITH_LDAP)
    find_package(Ldap REQUIRED)
    include_directories(${LDAP_INCLUDE_DIR})
	link_directories(${LDAP_PATH})
endif(WITH_LDAP)

# check if we need to enable db4
if(WITH_DB4)
	find_package(DB4 REQUIRED)
	include_directories(${DB4_INCLUDE_DIR})
	link_directories(${DB4_PATH})
endif(WITH_DB4)

# check out current version
set(THREE_PART_VERSION_REGEX "[0-9]+\\.[0-9]+\\.[0-9]+")
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/VERSION SMF_VERSION)

if(${SMF_VERSION} MATCHES ${THREE_PART_VERSION_REGEX})
    string(REGEX REPLACE "^([0-9]+)\\.[0-9]+\\.[0-9]+" "\\1" SMF_MAJOR_VERSION ${SMF_VERSION})
    string(REGEX REPLACE "^[0-9]+\\.([0-9])+\\.[0-9]+" "\\1" SMF_MINOR_VERSION ${SMF_VERSION})
    string(REGEX REPLACE "^[0-9]+\\.[0-9]+\\.([0-9]+)" "\\1" SMF_MICRO_VERSION ${SMF_VERSION})
else(${SMF_VERSION} MATCHES ${THREE_PART_VERSION_REGEX})
    message(FATAL_ERROR "Problem parsing version string, I can't parse it properly.")
endif(${SMF_VERSION} MATCHES ${THREE_PART_VERSION_REGEX})

math(EXPR version_number "${SMF_MAJOR_VERSION} * 1000000 + ${SMF_MINOR_VERSION} * 1000 + ${SMF_MICRO_VERSION}" )
set(SMF_VERSION_NUMBER ${version_number})

# write spmfilter.h
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter_config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter_config.h
)

# write spmfilter.pc
CONFIGURE_FILE(
	${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter.pc.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter.pc
	@ONLY
)

# check for enabled debugging
if(ENABLE_DEBUG)
	set(CMAKE_VERBOSE_MAKEFILE TRUE)
    add_definitions(-DDEBUG -g -O0 -Wall)
endif(ENABLE_DEBUG)

IF(APPLE)
	set(_link_flags "${_link_flags} -flat_namespace")
ENDIF(APPLE)

# build stuff in src directory
add_subdirectory(src)
add_subdirectory(man)

# * * * custom targets * * *
set(SMF_ARCHIVE_NAME "${CMAKE_PROJECT_NAME}-${SMF_VERSION}")

set(CPACK_PACKAGE_NAME "spmfilter")
set(CPACK_PACKAGE_VENDOR "spmfilter.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "spmfilter - mail filtering framework")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README)
set(CPACK_PACKAGE_VERSION ${SMF_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${SMF_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${SMF_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${SMF_MICROT_VERSION})
set(CPACK_COMPONENTS_ALL applications libraries headers)

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SMF_VERSION}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES
"\\\\.hg"
"nbproject"
"cmake_build"
"build\\\\.properties"
"spmfilter-Makefile\\\\.mk"
"spmfilter\\\\.pc$"
"spmfilter_config\\\\.h$"
)

include(CPack)