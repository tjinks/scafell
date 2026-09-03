add_library(Scafell
	err_handling.c
	list.c
	mmgt.c 
	str.c 
	ucdb.c
 )

target_link_libraries(Scafell PUBLIC compiler_flags)


