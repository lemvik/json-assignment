include_directories (.)

add_library (json_library
  json_sa.h
  json_sa.cpp
  )

add_executable (json main.cpp)
target_link_libraries (json json_library)
