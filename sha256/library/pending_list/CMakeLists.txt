add_library(pending_list STATIC pending_list.c)

target_include_directories(pending_list
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
