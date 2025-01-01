# prevent in-source builds
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory and run CMake from there.")
endif ()

cmake_minimum_required(VERSION 3.10)

project(clox)

set(VERSION "0.0.1")

set(CMAKE_CXX_STANDARD 90)

set(CMAKE_BINARY_DIR "${CMAKE_SOURCE_DIR}/build")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

if (CMAKE_BUILD_TYPE EQUAL "DEBUG" OR NOT DEFINED (CMAKE_BUILD_TYPE))
    set(
            DEBUG
            "DEBUG"
    )
endif ()

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
# set source code dir
file(GLOB_RECURSE LOX_SRC
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.c"
)
# set utf-8
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif ()

add_executable(${PROJECT_NAME} ${LOX_SRC})