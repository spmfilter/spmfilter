set(SPMFILTER_SRC
	main.c
)

add_executable(spmfilter ${SPMFILTER_SRC})
target_link_libraries(spmfilter esmtp glib-2.0)

