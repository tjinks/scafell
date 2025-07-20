add_library(scafell
	datum.c datum.h
	err_handling.c err_handling.h
	hash.c hash.h
	list.c list.h
	mmgt.c mmgt.h
	str.c str.h
	ucdb.c ucdb.h
	unicode_data.h
	os/osdefs.h
	os/osunix.h
	os/oswin.h
 )

target_link_libraries(scafell PUBLIC compiler_flags)


