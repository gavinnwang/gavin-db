message("hello world")
cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For clang-tidy.
set(BUILD_SHARED_LIBS OFF) # We expect external libraries to be linked statically.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(GavinDB
        VERSION 2024.03
        DESCRIPTION "Gavin's Relational Database Management System" 
        LANGUAGES C CXX
)

# if(NOT WIN32)
#   string(ASCII 27 Esc)
#   set(ColorReset   "${Esc}[m")
#   set(ColorBold    "${Esc}[1m")
#   set(ColorRed     "${Esc}[31m")
#   set(ColorGreen   "${Esc}[32m")
#   set(ColorYellow  "${Esc}[33m")
#   set(ColorBlue    "${Esc}[34m")
#   set(ColorMagenta "${Esc}[35m")
#   set(ColorCyan    "${Esc}[36m")
#   set(ColorWhite   "${Esc}[37m")
# endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
        message(STATUS "Setting build type to `Debug` as none was specified.")
        set(CMAKE_BUILD_TYPE "Debug")
endif()

message("project binary dir: " (${PROJECT_BINARY_DIR}))

file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" PATH_TO_CMAKELISTS_TXT)

if(EXISTS "${PATH_TO_CMAKELISTS_TXT}")
        message(FATAL_ERROR "Run CMake from a build subdirectory! \"mkdir build ; cd build ; cmake ..\" \
    Some junk files were created in this folder (CMakeCache.txt, CMakeFiles); you should delete those.")
endif()

message("Build mode: ${CMAKE_BUILD_TYPE}")

# Compiler flags.
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fsanitize=undefined")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Core
file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/src/*/*.cpp")

add_library(gavindb ${SOURCE_FILES})

# Shell
add_executable(gavindb_shell src/shell.cpp)
target_link_libraries(gavindb_shell gavindb)

# Include
set(SRC_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/include)
include_directories(${SRC_INCLUDE_DIR})


include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

include_directories(
  ${googletest_SOURCE_DIR}/googletest/include
  ${googletest_SOURCE_DIR}/googlemock/include
)

add_subdirectory(test)
