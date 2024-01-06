cmake_minimum_required(VERSION 3.12)
project(json-server-tests)

set(CMAKE_CXX_STANDARD 14)

include_directories(crow)

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests server Catch2::Catch2 httplib)

add_custom_target(run_tests COMMAND tests DEPENDS tests)
