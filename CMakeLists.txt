cmake_minimum_required(VERSION 3.15)

project(Scafell VERSION 0.1)
add_library(compiler_flags INTERFACE)
target_compile_features(compiler_flags INTERFACE c_std_11)

add_subdirectory(scuts)
add_subdirectory(scf-core)
add_subdirectory(scf-ucs)
