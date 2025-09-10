add_library(sha256_file STATIC sha256_file.c)

add_compile_options(-Wno-deprecated-declarations)

find_package(OpenSSL REQUIRED)


target_include_directories(sha256_file
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(sha256_file
  SYSTEM PUBLIC
    ${OPENSSL_INCLUDE_DIR}
)

target_link_libraries(sha256_file PUBLIC ${OPENSSL_LIBRARIES})
