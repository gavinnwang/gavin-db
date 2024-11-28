enable_testing()

include(GoogleTest)
file(GLOB_RECURSE GAVIN_DB_TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/*/*test.cpp")
message(STATUS "Discovered test sources: ${GAVIN_DB_TEST_SOURCES}")

set(TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test)

foreach(test_source ${GAVIN_DB_TEST_SOURCES})
  get_filename_component(test_name ${test_source} NAME_WE)
  add_executable(${test_name} ${test_source})

  target_include_directories(${test_name}
      PRIVATE
      ${PROJECT_SOURCE_DIR}/src/include
      ${TEST_INCLUDE_DIR}
      ${SQL_PARSER_LIBRARY_DIR}/src
  )
  target_link_libraries(${test_name} PRIVATE gavindb gtest_main fmt::fmt)
  add_dependencies(${test_name} sql_parser)

  gtest_discover_tests(${test_name})
endforeach ()
