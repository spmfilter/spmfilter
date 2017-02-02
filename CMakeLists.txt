cmake_minimum_required(VERSION 2.6)

project(spmfilter)

# include modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
include(${CMAKE_MODULE_PATH}/GNUInstallDirs.cmake)
include(FindPkgConfig)
include(SMFMacros)

# check for build.properties
include("${CMAKE_SOURCE_DIR}/build.properties" OPTIONAL)

# check prefix
if(PREFIX)
    SET(CMAKE_INSTALL_PREFIX ${PREFIX})
endif(PREFIX)

if(NOT TEST_NEXTHOP)
    SET(TEST_NEXTHOP localhost:25)
endif(NOT TEST_NEXTHOP)

# internal project settings
set(SMF_LIB_DIR "${CMAKE_INSTALL_FULL_LIBDIR}/spmfilter")
message(STATUS "using smf library dir ${SMF_LIB_DIR}")

# * * * find packages first * * *
find_package(Esmtp REQUIRED)
if(ESMTP_FOUND)
    include_directories(${ESMTP_INCLUDE_DIRS})
    link_directories(${ESMTP_LIBRARY_DIR})
endif(ESMTP_FOUND)

find_package(event REQUIRED)
if(EVENT_FOUND)
    include_directories(${EVENT_INCLUDE_DIRS})
    link_directories(${EVENT_LIBRARY_DIR})
endif(EVENT_FOUND)

pkg_search_module(LIBCMIME REQUIRED libcmime>=0.1.10)
if(LIBCMIME_FOUND)
    include_directories(${LIBCMIME_INCLUDE_DIRS})
    link_directories(${LIBCMIME_LIBRARY_DIRS})
endif(LIBCMIME_FOUND)

if(NOT WITHOUT_ZDB)
	message(STATUS "checking for one of the modules 'libzdb'")
	find_package(Zdb)
	include_directories(${ZDB_INCLUDE_DIR})
	link_directories(${ZDB_PATH})
endif(NOT WITHOUT_ZDB)

# check if we need to enable ldap
if(NOT WITHOUT_LDAP)
	message(STATUS "checking for one of the modules 'ldap'")
    find_package(Ldap)
    include_directories(${LDAP_INCLUDE_DIR})
	link_directories(${LDAP_PATH})
endif(NOT WITHOUT_LDAP)

# check if we need to enable db4
if(NOT WITHOUT_DB4)
	message(STATUS "checking for one of the modules 'db4'")
	find_package(DB4)
	include_directories(${DB4_INCLUDE_DIR})
	link_directories(${DB4_PATH})
endif(NOT WITHOUT_DB4)

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
set(SMF_SO_VERSION "${SMF_MAJOR_VERSION}.${SMF_MINOR_VERSION}")

# write spmfilter.h
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter_config.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/spmfilter_config.h
)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_params.h.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_params.h
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
    add_definitions(-DDEBUG -g -O0 -Wall -Werror)
endif(ENABLE_DEBUG)

IF(APPLE)
	set(_link_flags "${_link_flags} -flat_namespace")
ENDIF(APPLE)

# check for enabled testing
if (ENABLE_TESTING)
    ENABLE_TESTING()
    # for unit-testing you need the check-library
    message(STATUS "checking for test-modules")
    find_package(Check REQUIRED)
    include_directories(${CHECK_INCLUDES})
    add_subdirectory(test)
    file(COPY "${CMAKE_SOURCE_DIR}/test/samples" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/test")
endif(ENABLE_TESTING)

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
set(CPACK_PACKAGE_VERSION_PATCH ${SMF_MICRO_VERSION})
set(CPACK_COMPONENTS_ALL applications libraries headers)

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${SMF_VERSION}")
set(CPACK_SOURCE_GENERATOR TGZ)
set(CPACK_SOURCE_IGNORE_FILES
"\\\\.git"
"cmake_build"
"build"
"build\\\\.properties"
"spmfilter-Makefile\\\\.mk"
"spmfilter\\\\.pc$"
"spmfilter_config\\\\.h$"
"Vagrantfile"
"\\\\.idea"
"\\\\.vagrant"
)

include(CPack)
