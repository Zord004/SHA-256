add_library(cache STATIC cache.c)

target_include_directories(cache
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
