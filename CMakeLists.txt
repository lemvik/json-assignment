cmake_minimum_required (VERSION 3.8)

project (JSON)

set (CMAKE_CXX_STANDARD 17)

if ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR CMAKE_COMPILER_IS_GNUCXX) # Couldn't find variable for Clang, thus...
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic") 
elseif (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
endif ()

add_subdirectory(src)
add_subdirectory(test)

enable_testing()

add_test(NAME JSONText COMMAND json_test)
