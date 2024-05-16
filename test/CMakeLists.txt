enable_testing()

add_executable(
  hello_test 
  hello_test.cpp
)
add_executable(
  table_page_test 
  storage/table_page_test.cpp
)

target_link_libraries(
  hello_test
  GTest::gtest_main
)

target_link_libraries(
  table_page_test
  gavindb
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hello_test)
gtest_discover_tests(table_page_test)
