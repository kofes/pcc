cmake_minimum_required(VERSION 3.6)
project(compiler)
#the version number
set (COMPILER_VERSION_MAJOR 0)
set (COMPILER_VERSION_MINOR 3)

#configure a header file to pass some of CMake settings
#to the source code
configure_file (
    "${PROJECT_SOURCE_DIR}/CompilerConfig.cfg"
    "${PROJECT_BINARY_DIR}/CompilerConfig.hpp"
    )

#add the binary tree to the search path for include files
#so tha we wil find CompilerConfig.h
include_directories("${PROJECT_BINARY_DIR}")

#set standart c++11 and compilation's flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -Werror=return-type -ansi -pedantic -Werror=pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror=return-type -ansi -pedantic -Werror=pedantic")

#set source files
set(SOURCE_EXE main.cpp)

#add the executable
add_executable(${PROJECT_NAME} ${SOURCE_EXE})

#link to library
add_subdirectory(src)
target_link_libraries(${PROJECT_NAME} Compiler)

#add the install targets
install (TARGETS compiler DESTINATION bin)
install (FILES "${PROJECT_BINARY_DIR}/CompilerConfig.hpp" DESTINATION include)
