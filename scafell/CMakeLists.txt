add_library(Scafell
	datum.c
	err_handling.c
	hash.c
	list.c 
	mmgt.c 
	str.c 
	ucdb.c 
 )

target_link_libraries(Scafell PUBLIC compiler_flags)


