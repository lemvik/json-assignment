include_directories (.)

add_library (json_library
  json.h
  json.cpp
  )

add_executable (json main.cpp)
target_link_libraries (json json_library)
