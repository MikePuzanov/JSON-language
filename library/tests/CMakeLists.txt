cmake_minimum_required(VERSION 3.12)
project(tests)

set(CMAKE_CXX_STANDARD 14)


include(FetchContent)

FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG v0.9.5
)

FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0 
)

FetchContent_MakeAvailable(Catch2)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(TEST_SOURCES
    main_test.cpp
    ../src/json-language.cpp
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain httplib)

add_custom_target(run_tests COMMAND tests DEPENDS tests)
