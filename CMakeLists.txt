cmake_minimum_required (VERSION 3.8)

project (JSON)

set (CMAKE_CXX_STANDARD 14)

if ((CMAKE_CXX_COMPILER_ID MATCHES "Clang") OR CMAKE_COMPILER_IS_GNUCXX) # Couldn't find variable for Clang, thus...
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wpedantic") 
elseif (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Wall")
  include_directories ("c:/boost_1_64_0")
  link_directories ("c:/boost_1_64_0/stage/lib")
  set (Boost_DEBUG 1)
  set (BOOST_ROOT "c:/boost_1_64_0")
  set (BOOST_LIBRARYDIR "c:/boost_1_64_0/stage/lib")
endif ()

add_subdirectory(src)
add_subdirectory(test)

enable_testing()

add_test(NAME json_test COMMAND json_test)
