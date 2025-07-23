add_library(scuts
	scuts.c scuts.h
 )

target_link_libraries(scuts PUBLIC compiler_flags)

