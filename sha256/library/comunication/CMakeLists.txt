add_library(comunication STATIC comunication.c)

target_include_directories(comunication
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
