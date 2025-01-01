# prevent in-source builds
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build directory and run CMake from there.")
endif ()

cmake_minimum_required(VERSION 3.10)

project(clox)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

set(
        CMAKE_SOURCE_DIR
        src/main.c
        src/vm.c
        src/scanner.c
        src/compiler.c
        src/debug.c
        src/memory.c
        src/value.c
        src/chunk.c
        src/object.c
        src/table.c
)
# set utf-8
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/source-charset:utf-8 /execution-charset:utf-8)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-finput-charset=UTF-8 -fexec-charset=UTF-8)
endif ()

add_executable(${PROJECT_NAME} ${CMAKE_SOURCE_DIR})