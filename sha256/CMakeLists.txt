cmake_minimum_required(VERSION 3.20)

project(SHA256 VERSION 1.0)

set(CMAKE_MACOSX_RPATH 1)
set(BUILD_SHARED_LIBS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_compile_options(-Wno-deprecated-declarations)

add_subdirectory(library/sha256_file)
add_subdirectory(library/comunication)
add_subdirectory(library/cache)
add_subdirectory(library/pending_list)

add_executable(server src/server.c)
add_executable(client src/client.c)

target_link_libraries(server PRIVATE sha256_file)
target_link_libraries(client PRIVATE sha256_file)
target_link_libraries(server PRIVATE comunication)
target_link_libraries(client PRIVATE comunication)
target_link_libraries(server PRIVATE cache)
target_link_libraries(server PRIVATE pending_list)
