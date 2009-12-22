# find ldap library
#

find_library(HAVE_LDAP ldap)
if(HAVE_LDAP-NOTFOUND)
	message(FATAL_ERROR "ldap has been enabled but library coud not be found")
else(HAVE_LDAP-NOTFOUND)
	get_filename_component(LDAP_PATH "${HAVE_LDAP}" PATH)
	string(REGEX REPLACE "lib$" "include" LDAP_INCLUDE_PATH ${LDAP_PATH})
	message(STATUS "Found ldap library ${HAVE_LDAP}")
	message(STATUS "Found ldap include path ${LDAP_INCLUDE_PATH}")
endif(HAVE_LDAP-NOTFOUND)
