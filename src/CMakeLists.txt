set(SPMFILTER_SRC
    main.c
)

set(LIB_SMF_SRC
    utils.c
    trace.c
    smtp.c
    smtp_codes.c
	settings.c
	group_settings.c
	message.c
	lookup.c
	lookup_result.c
)

set(COMMON_LIBS m esmtp ${GLIB2_LIBRARIES} ${GMODULE_LIBRARIES} ${GMIME_LIBRARIES})
if(HAVE_ZDB)
    list(APPEND COMMON_LIBS zdb)
    list(APPEND LIB_SMF_SRC sql_lookup.c)
endif(HAVE_ZDB)

if(HAVE_LDAP)
    list(APPEND COMMON_LIBS ldap lber)
    list(APPEND LIB_SMF_SRC ldap_lookup.c)
endif(HAVE_LDAP)

list(REMOVE_DUPLICATES COMMON_LIBS)

add_library(smf SHARED ${LIB_SMF_SRC})
target_link_libraries(smf ${COMMON_LIBS})
set_property(TARGET smf PROPERTY PUBLIC_HEADER spmfilter.h)
set_property(TARGET smf PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smf PROPERTY SOVERSION ${SMF_VERSION})
set_property(SOURCE spmfilter.h PROPERTY GENERATED TRUE)
set_property(SOURCE spmfilter.pc PROPERTY GENERATED TRUE)

add_library(smtpd SHARED smtpd.c mailconn.c)
set_property(TARGET smtpd PROPERTY VERSION ${SMF_VERSION})
set_property(TARGET smtpd PROPERTY SOVERSION ${SMF_VERSION})
target_link_libraries(smtpd ${COMMON_LIBS} smf)

add_library(pipe SHARED pipe.c mailconn.c)
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

