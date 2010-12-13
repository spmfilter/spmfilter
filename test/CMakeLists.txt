if(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB214_LIBRARIES})
else(HAVE_GLIB214)
	list(APPEND COMMON_LIBS ${GLIB2_LIBRARIES})
endif(HAVE_GLIB214)

add_executable(test_envelope test_envelope.c)
target_link_libraries(test_envelope smf ${COMMON_LIBS})

ADD_TEST(SMFEnvelope ${EXECUTABLE_OUTPUT_PATH}/test_envelope)