add_library(Scafell
	datum.c #datum.h
	err_handling.c
 )

target_link_libraries(Scafell PUBLIC compiler_flags)


