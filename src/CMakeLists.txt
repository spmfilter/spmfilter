set(SPMFILTER_SRC
	main.c
)

set(LIB_SMF_SRC
	spmutils.c
	trace.c
	smtp.c
	sql_lookup.c
	ldap_lookup.c
)

SET(COMMON_LIBS m esmtp ${GLIB2_LIBRARIES} ${GMIME_LIBRARIES})

if(HAVE_LDAP)
	list(APPEND COMMON_LIBS ldap)
endif(HAVE_LDAP)

list(REMOVE_DUPLICATES COMMON_LIBS)

add_library(smf SHARED ${LIB_SMF_SRC})
target_link_libraries(smf ${COMMON_LIBS})

add_library(smtpd SHARED smtpd.c)
target_link_libraries(smtpd ${COMMON_LIBS} smf)

add_library(pipe SHARED pipe.c)
target_link_libraries(pipe ${COMMON_LIBS} smf)

add_executable(spmfilter ${SPMFILTER_SRC})
target_link_libraries(spmfilter ${COMMON_LIBS} smf)

install(TARGETS smf smtpd pipe spmfilter 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib/spmfilter
)

