cmake_minimum_required(VERSION 3.6)
project(compiler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

set(SOURCE_EXE main.cpp)

include_directories(inc)

add_executable(${PROJECT_NAME} ${SOURCE_EXE})

add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} Compiler)