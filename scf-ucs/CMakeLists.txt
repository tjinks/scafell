add_library(scf-ucs
	unicode_data.h
	ucsdb.c ucsdb.h
    ucs_string.c ucs_string.h
    codecs.c codecs.h
 )

target_link_libraries(scf-ucs PUBLIC compiler_flags)
target_link_libraries(scf-ucs PUBLIC scf-core)
target_include_directories(scf-ucs PUBLIC "${PROJECT_SOURCE_DIR}/scf-core")

add_subdirectory(scf-ucs-tests)

