cmake_minimum_required(VERSION 3.12)
project(json-server)

set(CMAKE_CXX_STANDARD 14)

include_directories(crow)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(json-server ${SOURCES})

add_subdirectory(tests)
