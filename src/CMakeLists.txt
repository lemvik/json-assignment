include_directories (.)

add_library (json_library
  json_sa.h
  json_sa.cpp
  json_parser.h
  json_parser.cpp
  json.h
  json.cpp
  utils.h
  utils.cpp
  )

add_executable (json main.cpp)
target_link_libraries (json json_library)
