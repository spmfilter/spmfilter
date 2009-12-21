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

add_library(smf SHARED ${LIB_SMF_SRC})
target_link_libraries(smf
	esmtp 
	${GLIB2_LIBRARIES}
	${GMIME_LIBRARIES}
)

add_executable(spmfilter ${SPMFILTER_SRC})
target_link_libraries(spmfilter esmtp glib-2.0 gmime-2.4 smf)

