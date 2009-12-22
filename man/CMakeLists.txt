include(SMFMacros)

set(MAN_SRC
	spmfilter.1
	spmfilter.conf.5)
	
foreach(man ${MAN_SRC})
	create_manpage(${man})
endforeach(man)

