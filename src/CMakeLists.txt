set(SPMFILTER_SRC
    main.c
)

set(LIB_SMF_SRC
    smf_utils.c
    smf_trace.c
    smf_smtp.c
    smf_smtp_codes.c
	smf_settings.c
	smf_group_settings.c
	smf_message.c
	smf_lookup.c
	smf_lookup_result.c
	smf_platform.c
	smf_core.c
)

set(COMMON_LIBS m esmtp ${GMODULE_LIBRARIES} ${GMIME_LIBRARIES})

if(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB214_LIBRARIES})
else(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB2_LIBRARIES})
endif(HAVE_GLIB214)

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
	list(APPEND LIB_SMF_SRC smf_db4_lookup.c)
endif(HAVE_DB4)

#if(HAVE_PCRE)
#	list(APPEND COMMON_LIBS ${PCRE_LIBRARIES})
#endif(HAVE_PCRE)

list(REMOVE_DUPLICATES COMMON_LIBS)

add_library(smf SHARED ${LIB_SMF_SRC})
target_link_libraries(smf ${COMMON_LIBS})
set_property(TARGET smf PROPERTY PUBLIC_HEADER spmfilter.h)
set_property(TARGET smf PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smf PROPERTY SOVERSION ${SMF_VERSION})
set_property(SOURCE spmfilter.h PROPERTY GENERATED TRUE)
set_property(SOURCE spmfilter.pc PROPERTY GENERATED TRUE)

add_library(smtpd SHARED smf_smtpd.c smf_mailconn.c)
set_property(TARGET smtpd PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smtpd PROPERTY SOVERSION ${SMF_VERSION})
target_link_libraries(smtpd ${COMMON_LIBS} smf)

add_library(pipe SHARED smf_pipe.c smf_mailconn.c)
set_property(TARGET pipe PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET pipe PROPERTY SOVERSION ${SMF_VERSION})
target_link_libraries(pipe ${COMMON_LIBS} smf)

add_executable(spmfilter ${SPMFILTER_SRC})
target_link_libraries(spmfilter ${GLIB2_LIBRARIES} ${GMODULE_LIBRARIES} smf)

install(TARGETS smf smtpd pipe spmfilter
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib/spmfilter
    PUBLIC_HEADER DESTINATION include
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/spmfilter.pc
	DESTINATION lib/pkgconfig
)
