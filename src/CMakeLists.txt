cmake_minimum_required(VERSION 3.6)
project(Compiler)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")

file(GLOB SOURCE_LIB "*.cpp")

add_library(Compiler SHARED ${SOURCE_LIB})
