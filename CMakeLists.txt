message("hello world")
cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(GavinDB
        VERSION 2024.03
        DESCRIPTION "Gavin's Relational Database Management System" 
        LANGUAGES C CXX
)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to `Debug` as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug")
endif()

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)

if(EXISTS "${PATH_TO_CMAKELISTS_TXT}")
        message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# Compiler flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined -fno-sanitize=alignment")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Core
# Bad practice but convinient
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*/*.cpp")
add_library(gavindb ${SOURCE_FILES})

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(fmt)


include(ExternalProject)
ExternalProject_Add(sql_parser
    GIT_REPOSITORY git@github.com:hyrise/sql-parser.git
    GIT_TAG master
    PREFIX ${CMAKE_BINARY_DIR}/external/sql-parser
    BUILD_IN_SOURCE 1
    CONFIGURE_COMMAND "make"
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS "${CMAKE_BINARY_DIR}/external/sql-parser/src/sql_parser/libsqlparser.so"
)

ExternalProject_Get_Property(sql_parser BINARY_DIR)
set(SQL_PARSER_LIBRARY_DIR "${BINARY_DIR}")
message(STATUS "Sql_parser location: ${SQL_PARSER_LIBRARY_DIR}/libsqlparser.so")

add_custom_command(
    TARGET sql_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BINARY_DIR}/libsqlparser.so"
            "${CMAKE_BINARY_DIR}/test/"
    VERBATIM
)
add_custom_command(
    TARGET sql_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BINARY_DIR}/libsqlparser.so"
            "${CMAKE_BINARY_DIR}/bin/"
    VERBATIM
)

add_custom_command(
    TARGET sql_parser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${BINARY_DIR}/libsqlparser.so"
            "${CMAKE_BINARY_DIR}/"
    VERBATIM
)

target_link_libraries(gavindb PRIVATE fmt::fmt "${SQL_PARSER_LIBRARY_DIR}/libsqlparser.so")

# Include
target_include_directories(gavindb PRIVATE 
${PROJECT_SOURCE_DIR}/src/include
${SQL_PARSER_LIBRARY_DIR}/src
)

add_dependencies(gavindb sql_parser)

add_subdirectory(test)
