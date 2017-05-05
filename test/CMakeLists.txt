find_package (Boost COMPONENTS unit_test_framework REQUIRED)
include_directories (
  ${JSON_SOURCE_DIR}/src
  ${Boost_SOURCE_DIR}
  )
add_definitions (-DBOOST_TEST_DYN_LINK)
add_executable (json_test
                test_main.cpp
                json_test.cpp
                json_parse_test.cpp)
target_link_libraries (json_test json_library ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
