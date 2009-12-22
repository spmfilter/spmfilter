include(FindGzip)

macro(CREATE_MANPAGE _manpage)
	get_filename_component(_input ${_manpage} ABSOLUTE)
	get_filename_component(_base ${_input} NAME)
	
	string(REGEX MATCH "([0-9])$" _section ${_base})
	
	set(_compressed ${CMAKE_CURRENT_BINARY_DIR}/${_base}.gz)

	add_custom_target(${_manpage} ALL 
		COMMAND ${GZIP_TOOL} -c ${_input} > ${_compressed}  
		DEPENDS ${_input} 
		COMMENT "Compressing ${_base}"
	)
	
	install(FILES ${_compressed} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man${_section})
	
endmacro(CREATE_MANPAGE)