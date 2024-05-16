enable_testing()

# add_executable(
#   hello_test 
#   hello_test.cpp
# )
# add_executable(
#   table_page_test 
#   storage/table_page_test.cpp
# )
#
# target_link_libraries(
#   hello_test
#   GTest::gtest_main
# )
#
# target_link_libraries(
#   table_page_test
#   gavindb
#   GTest::gtest_main
# )

include(GoogleTest)
file(GLOB_RECURSE GAVIN_DB_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")

foreach(test_source ${GAVIN_DB_TEST_SOURCES})
  get_filename_component(test_name ${test_source} NAME_WE)

  add_executable(${test_name} ${test_source})
  target_link_libraries(${test_name} gavindb GTest::gtest_main)
  gtest_discover_tests(${test_name})
  
endforeach ()
# gtest_discover_tests(hello_test)
# gtest_discover_tests(table_page_test)
