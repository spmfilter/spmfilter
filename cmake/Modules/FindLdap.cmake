# find ldap library
#

find_library(HAVE_LDAP ldap)
if(HAVE_LDAP)
	get_filename_component(LDAP_PATH "${HAVE_LDAP}" PATH)
	string(REGEX REPLACE "lib$" "include" LDAP_INCLUDE_PATH ${LDAP_PATH})
	message(STATUS "  found ldap library ${HAVE_LDAP}")
	message(STATUS "  found ldap include path ${LDAP_INCLUDE_PATH}")
else(HAVE_LDAP)
	message(STATUS "  ldap library could not be found, disabling ldap backend.")
endif(HAVE_LDAP)
