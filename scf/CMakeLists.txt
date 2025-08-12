add_library(scf
	scf.c scf.h
 )

target_link_libraries(scf PUBLIC compiler_flags)
target_link_libraries(scf PUBLIC scf-core)
target_link_libraries(scf PUBLIC scf-ucs)
target_include_directories(scf PUBLIC "${PROJECT_SOURCE_DIR}/scf-core")
target_include_directories(scf PUBLIC "${PROJECT_SOURCE_DIR}/scf-ucs")


