cmake_minimum_required(VERSION 3.12)
project(json-server)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(json-server ${SOURCES})

target_include_directories(json-server PRIVATE ${nlohmann_json_SOURCE_DIR}/single_include)

add_subdirectory(tests)

target_include_directories(tests PRIVATE ${nlohmann_json_SOURCE_DIR}/single_include)