add_executable(scafell_tests
	main.c
	buffer_tests.c
	hash_tests.c
	string_tests.c
	list_tests.c
	mmgt_tests.c
 )

target_link_libraries(scafell_tests PUBLIC compiler_flags)
target_link_libraries(scafell_tests PUBLIC scafell)
target_link_libraries(scafell_tests PUBLIC scuts)
target_include_directories(scafell_tests PUBLIC "${PROJECT_SOURCE_DIR}/scafell")
target_include_directories(scafell_tests PUBLIC "${PROJECT_SOURCE_DIR}/scuts")


