list(APPEND COMMON_LIBS ${GLIB2_LIBRARIES})

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

add_executable(test_header test_header.c)
target_link_libraries(test_header smf ${COMMON_LIBS})
ADD_TEST(SMFHeader_T ${EXECUTABLE_OUTPUT_PATH}/test_header)

add_executable(test_part test_part.c)
target_link_libraries(test_part smf ${COMMON_LIBS})
ADD_TEST(SMFPart_T ${EXECUTABLE_OUTPUT_PATH}/test_part)

add_executable(test_message test_message.c)
target_link_libraries(test_message smf ${COMMON_LIBS})
ADD_TEST(SMFMessage_T ${EXECUTABLE_OUTPUT_PATH}/test_message)

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
