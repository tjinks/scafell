add_executable(scf-core-tests
	main.c
	buffer_tests.c
	hash_tests.c
	string_tests.c
	list_tests.c
	mmgt_tests.c
 )

target_link_libraries(scf-core-tests PUBLIC compiler_flags)
target_link_libraries(scf-core-tests PUBLIC scf-core)
target_link_libraries(scf-core-tests PUBLIC scuts)
target_include_directories(scf-core-tests PUBLIC "${PROJECT_SOURCE_DIR}/scf-core")
target_include_directories(scf-core-tests PUBLIC "${PROJECT_SOURCE_DIR}/scuts")


