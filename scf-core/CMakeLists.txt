add_library(scf-core
	datum.c datum.h
	err_handling.c err_handling.h
	hash.c hash.h
	list.c list.h
	mmgt.c mmgt.h
    machine_info.c machine_info.h
    
	osdefs.h
	osunix.h
	oswin.h
	fs.h
 )

target_link_libraries(scf-core PUBLIC compiler_flags)

add_subdirectory(scf-core-tests)

