set(SPMFILTER_SRC
	main.c
	smf_server.c
)

set(LIB_SMF_SRC
	smf_core.c
	smf_dict.c
	smf_envelope.c
	smf_header.c
	smf_internal.c
	smf_list.c
	smf_nexthop.c
	smf_md5.c
	smf_message.c
	smf_modules.c
	smf_part.c
	smf_session.c
	smf_settings.c
	smf_smtp.c
	smf_trace.c
	smf_email_address.c
)

set(COMMON_LIBS m esmtp dl ${LIBCMIME_LIBRARIES})

if(HAVE_ZDB)
	list(APPEND COMMON_LIBS zdb)
	list(APPEND LIB_SMF_SRC smf_lookup_sql.c)
endif(HAVE_ZDB)

if(HAVE_LDAP)
	list(APPEND COMMON_LIBS ldap lber)
	list(APPEND LIB_SMF_SRC smf_lookup_ldap.c)
endif(HAVE_LDAP)

if(HAVE_DB4)
	list(APPEND COMMON_LIBS db)
	list(APPEND LIB_SMF_SRC smf_lookup_db4.c)
endif(HAVE_DB4)

list(REMOVE_DUPLICATES COMMON_LIBS)

add_library(smf SHARED ${LIB_SMF_SRC})
target_link_libraries(smf ${COMMON_LIBS})
set_property(TARGET smf PROPERTY PUBLIC_HEADER
	spmfilter.h spmfilter_config.h)

set_property(TARGET smf PROPERTY PRIVATE_HEADER
	smf_core.h
	smf_dict.h
	smf_email_address.h
	smf_envelope.h
	smf_header.h
	smf_list.h
	smf_lookup.h
	smf_message.h
	smf_part.h
	smf_session.h
	smf_settings.h
	smf_smtp.h
	smf_trace.h
)

set_property(TARGET smf PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smf PROPERTY SOVERSION ${SMF_VERSION})
set_property(SOURCE spmfilter_config.h PROPERTY GENERATED TRUE)
set_property(SOURCE spmfilter.pc PROPERTY GENERATED TRUE)

add_library(smtpd SHARED smf_smtpd.c smf_session.c)
set_property(TARGET smtpd PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smtpd PROPERTY SOVERSION ${SMF_VERSION})
set_property(TARGET smtpd PROPERTY LINK_FLAGS ${_link_flags})
target_link_libraries(smtpd ${COMMON_LIBS} smf)

add_library(pipe SHARED smf_pipe.c smf_session.c)
set_property(TARGET pipe PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET pipe PROPERTY SOVERSION ${SMF_VERSION})
set_property(TARGET pipe PROPERTY LINK_FLAGS ${_link_flags})
target_link_libraries(pipe ${COMMON_LIBS} smf)

add_executable(spmfilter ${SPMFILTER_SRC})
target_link_libraries(spmfilter smf)

install(TARGETS smf smtpd pipe spmfilter
	RUNTIME DESTINATION sbin
	LIBRARY DESTINATION ${LIBDIR}/spmfilter
	PUBLIC_HEADER DESTINATION include/spmfilter
	PRIVATE_HEADER DESTINATION include/spmfilter/smf
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/spmfilter.pc
	DESTINATION ${LIBDIR}/pkgconfig
)
