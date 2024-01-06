cmake_minimum_required(VERSION 3.12)
project(json-server-tests)

set(CMAKE_CXX_STANDARD 14)

include_directories(crow)

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")

# Выводим список исходных файлов для отладки
message("TEST_SOURCES: ${TEST_SOURCES}")

add_executable(tests ${TEST_SOURCES})

target_link_libraries(tests server Catch2::Catch2 httplib)

add_custom_target(run_tests COMMAND tests DEPENDS tests)
