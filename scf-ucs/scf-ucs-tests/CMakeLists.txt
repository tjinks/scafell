add_executable(scf-ucs-tests
	main.c
    ucsdb_tests.c
 )

target_link_libraries(scf-ucs-tests PUBLIC compiler_flags)
target_link_libraries(scf-ucs-tests PUBLIC scf-ucs)
target_link_libraries(scf-ucs-tests PUBLIC scuts)
target_include_directories(scf-ucs-tests PUBLIC "${PROJECT_SOURCE_DIR}/scf-ucs")
target_include_directories(scf-ucs-tests PUBLIC "${PROJECT_SOURCE_DIR}/scuts")


