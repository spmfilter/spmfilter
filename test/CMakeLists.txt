list(APPEND COMMON_LIBS)

add_executable(test_settings test_settings.c)
target_link_libraries(test_settings smf ${COMMON_LIBS})
ADD_TEST(SMFSettings_T ${EXECUTABLE_OUTPUT_PATH}/test_settings)

add_executable(test_session test_session.c)
target_link_libraries(test_session smf ${COMMON_LIBS})
ADD_TEST(SMFSession_T ${EXECUTABLE_OUTPUT_PATH}/test_session)

add_executable(test_envelope test_envelope.c)
target_link_libraries(test_envelope smf ${COMMON_LIBS})
ADD_TEST(SMFEnvelope_T ${EXECUTABLE_OUTPUT_PATH}/test_envelope)

add_executable(test_ea test_ea.c)
target_link_libraries(test_ea smf ${COMMON_LIBS})
ADD_TEST(SMFEmailAddress_T ${EXECUTABLE_OUTPUT_PATH}/test_ea)

add_executable(test_list test_list.c)
target_link_libraries(test_list smf ${COMMON_LIBS})
ADD_TEST(SMFList_T ${EXECUTABLE_OUTPUT_PATH}/test_list)

add_executable(test_dict test_dict.c)
target_link_libraries(test_dict smf ${COMMON_LIBS})
ADD_TEST(SMFDict_T ${EXECUTABLE_OUTPUT_PATH}/test_dict)

add_executable(test_header test_header.c)
target_link_libraries(test_header smf ${COMMON_LIBS})
ADD_TEST(SMFHeader_T ${EXECUTABLE_OUTPUT_PATH}/test_header)

add_executable(test_part test_part.c)
target_link_libraries(test_part smf ${COMMON_LIBS})
ADD_TEST(SMFPart_T ${EXECUTABLE_OUTPUT_PATH}/test_part)

add_executable(test_message test_message.c)
target_link_libraries(test_message smf ${COMMON_LIBS})
ADD_TEST(SMFMessage_T ${EXECUTABLE_OUTPUT_PATH}/test_message)

add_executable(test_core test_core.c)
target_link_libraries(test_core smf ${COMMON_LIBS})
ADD_TEST(smf_core ${EXECUTABLE_OUTPUT_PATH}/test_core)

add_executable(test_smtp test_smtp.c)
target_link_libraries(test_smtp smf ${COMMON_LIBS})
ADD_TEST(smf_core ${EXECUTABLE_OUTPUT_PATH}/test_smtp)

add_executable(test_engine_pipe test_engine_pipe.c)
target_link_libraries(test_engine_pipe smf pipe ${COMMON_LIBS})
#ADD_TEST(SMFSettings_T ${EXECUTABLE_OUTPUT_PATH}/test_engine_pipe)
add_executable(test_engine_pipe_execute test_engine_pipe_execute.c)
ADD_TEST(test_engine_pipe_execute ${EXECUTABLE_OUTPUT_PATH}/test_engine_pipe_execute)

target_link_libraries(test_core smf ${COMMON_LIBS})
ADD_TEST(smf_core ${EXECUTABLE_OUTPUT_PATH}/test_core)

if(HAVE_DB4)
	add_executable(test_db4 test_db4.c)
	target_link_libraries(test_db4 smf ${COMMON_LIBS} db)
	ADD_TEST(DB4 ${EXECUTABLE_OUTPUT_PATH}/test_db4)
endif(HAVE_DB4)

if(HAVE_LDAP)
	add_executable(test_lookup_ldap test_lookup_ldap.c)
        target_link_libraries(test_lookup_ldap smf ${COMMON_LIBS} ldap)
        ADD_TEST(LDAP ${EXECUTABLE_OUTPUT_PATH}/test_lookup_ldap)
endif(HAVE_LDAP)

if(HAVE_ZDB)
	add_executable(test_lookup_sql test_lookup_sql.c)
        target_link_libraries(test_lookup_sql smf ${COMMON_LIBS} zdb)
        ADD_TEST(SQL ${EXECUTABLE_OUTPUT_PATH}/test_lookup_sql)
	add_executable(test_lookup_sqlite test_lookup_sqlite.c)
        target_link_libraries(test_lookup_sqlite smf ${COMMON_LIBS} zdb)
        ADD_TEST(SQL ${EXECUTABLE_OUTPUT_PATH}/test_lookup_sqlite)
endif(HAVE_ZDB)
