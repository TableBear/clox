cmake_minimum_required(VERSION 3.10)

project(clox)

include_directories(${PROJECT_SOURCE_DIR}/include)

set(
    CMAKE_SOURCE_DIR 
    src/main.c 
    src/debug.c
    src/memory.c 
    src/value.c 
    src/chunk.c
)

add_executable(clox ${CMAKE_SOURCE_DIR})