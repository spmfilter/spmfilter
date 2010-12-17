if(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB214_LIBRARIES})
else(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB2_LIBRARIES})
endif(HAVE_GLIB214)

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

